#include "deliverable/renderer/ForwardRenderer.h"
#include "deliverable/renderer/DeferredRenderer.h"
#include "deliverable/renderer/ForwardPlusRenderer.h"
#include "deliverable/renderer/TiledDeferredRenderer.h"
#include "deliverable/renderer/ImGuiRenderer.h"
#include "deliverable/Window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <random>
#include <fstream>
#include <string>

using namespace deliverable;

#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

static const int MAX_FRAMES_TO_CAPTURE = 1000;

static const unsigned DEFAULT_SEED = 1000;

int lightCount = 0;
int objectCountX = 0;
int objectCountY = 0;

std::vector<Light> lights;
std::vector<RenderCommand> renderCommands;

float phi = 1.2f;
float theta = 4.0f;

bool capturingFrameTimes = false;
std::vector<double> frameTimes;

const char* renderPipelines[] = { "Forward", "Deferred", "Tiled Forward", "Tiled Deferred" };
unsigned currentRenderPipeline = 0;

std::string fpsString;
std::string frametimeString;

Mesh* LoadModel(const std::string& filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open())
		return nullptr;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;

	std::vector<float> vertexData;
	std::vector<unsigned> indices;
	unsigned modelIndexCount = 0;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.find("v ") != std::string::npos)
		{
			glm::vec3 position;
			sscanf(line.c_str(), "v %f %f %f", &position[0], &position[1], &position[2]);
			positions.push_back(position);
		}
		else if (line.find("vn ") != std::string::npos)
		{
			glm::vec3 normal;
			sscanf(line.c_str(), "vn %f %f %f", &normal[0], &normal[1], &normal[2]);
			normals.push_back(normal);
		}
		else if (line.find("f ") != std::string::npos)
		{
			glm::uvec3 positionIndices;
			glm::uvec3 normalIndices;
			sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &positionIndices[0], &normalIndices[0], &positionIndices[1], &normalIndices[1], &positionIndices[2], &normalIndices[2]);

			for (unsigned i = 0; i < 3; i++)
			{
				indices.push_back(modelIndexCount++);

				glm::vec3 position = positions[positionIndices[i] - 1];
				glm::vec3 normal = normals[normalIndices[i] - 1];
				vertexData.push_back(position[0]);
				vertexData.push_back(position[1]);
				vertexData.push_back(position[2]);
				vertexData.push_back(normal[0]);
				vertexData.push_back(normal[1]);
				vertexData.push_back(normal[2]);
			}
		}
	}

	file.close();

	unsigned modelVertexArrayHandle;
	glGenVertexArrays(1, &modelVertexArrayHandle);
	glBindVertexArray(modelVertexArrayHandle);

	unsigned modelVertexBufferHandle;
	glGenBuffers(1, &modelVertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, modelVertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	unsigned modelIndexBufferHandle;
	glGenBuffers(1, &modelIndexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelIndexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	return new Mesh{ modelVertexArrayHandle, modelIndexCount };
}

unsigned CreateSphereMesh(unsigned slices, unsigned stacks)
{
	std::vector<float> vertexData;
	std::vector<unsigned> indices;

	for (unsigned i = 0; i <= stacks; ++i) {
		float v = static_cast<float>(i) / stacks;
		float theta = TWO_PI * v;

		for (unsigned j = 0; j <= slices; j++) {
			float u = static_cast<float>(j) / slices;
			float phi = TWO_PI * u;

			float x = sinf(phi) * cosf(theta);
			float y = cosf(phi);
			float z = sinf(phi) * sinf(theta);

			for (unsigned k = 0; k < 2; k++)
			{
				vertexData.push_back(x);
				vertexData.push_back(y);
				vertexData.push_back(z);
			}
		}
	}

	for (unsigned i = 0; i < stacks * slices + slices; ++i)
	{
		indices.push_back(i);
		indices.push_back(i + slices + 1);
		indices.push_back(i + slices);

		indices.push_back(i + slices + 1);
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	unsigned sphereVertexArrayHandle;
	glGenVertexArrays(1, &sphereVertexArrayHandle);
	glBindVertexArray(sphereVertexArrayHandle);

	unsigned sphereVertexBufferHandle;
	glGenBuffers(1, &sphereVertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	unsigned sphereIndexBufferHandle;
	glGenBuffers(1, &sphereIndexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	return sphereVertexArrayHandle;
}

unsigned CreateScreenSpaceQuadMesh()
{
	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	unsigned indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	unsigned quadVertexArrayHandle;
	glGenVertexArrays(1, &quadVertexArrayHandle);
	glBindVertexArray(quadVertexArrayHandle);

	unsigned quadVertexBufferHandle;
	glGenBuffers(1, &quadVertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)(0 * sizeof(float)));

	unsigned quadIndexBufferHandle;
	glGenBuffers(1, &quadIndexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	return quadVertexArrayHandle;
}

void OnObjectCountChanged(Mesh* bunnyMesh)
{
	renderCommands.clear();

	for (int j = 0; j < objectCountY; j++)
	{
		for (int i = 0; i < objectCountX; i++)
		{
			float x = static_cast<float>(-((objectCountX / 2.0f) * 2.0f) + 1.0f + (i * 2.0f));
			float z = static_cast<float>(-((objectCountY / 2.0f) * 2.0f) + 1.0f + (j * 2.0f));

			glm::vec3 position(x, 0, z);
			glm::mat4 modelMatrix(1.0f);
			modelMatrix = glm::translate(modelMatrix, position);
			modelMatrix = glm::rotate(modelMatrix, 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f));
			renderCommands.push_back({ modelMatrix, bunnyMesh });
		}
	}
}

void OnLightCountChanged()
{
	std::mt19937 gen;
	gen.seed(DEFAULT_SEED);
	std::uniform_real_distribution<> dis(0.0f, 1.0f);

	float minX = static_cast<float>(-objectCountX);
	float maxX = static_cast<float>(objectCountX);
	float minY = -1.2f;
	float maxY = 1.2f;
	float minZ = static_cast<float>(-objectCountY);
	float maxZ = static_cast<float>(objectCountY);

	lights.clear();

	for (int i = 0; i < lightCount; ++i)
	{
		float x = static_cast<float>(dis(gen)) * (maxX - minX) + minX;
		float y = static_cast<float>(dis(gen)) * (maxY - minY) + minY;
		float z = static_cast<float>(dis(gen)) * (maxZ - minZ) + minZ;

		Light light;
		light.positionAndRadius = glm::vec4(x, y, z, 1.0f);
		light.color = glm::vec4(dis(gen) * 5.0f, dis(gen) * 5.0f, dis(gen) * 5.0f, 1.0f);
		lights.push_back(light);
	}
}

int main()
{
	Window* window = new Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Situational Analysis of Modern Rendering Techniques for Real-time 3D Graphics");

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(65.0f), window->GetWidth() / static_cast<float>(window->GetHeight()), 1.0f, 1024.0f);

	unsigned lightSphereStacks = 6;
	unsigned lightSphereSlices = 6;
	unsigned lightSphereVertexArrayHandle = CreateSphereMesh(lightSphereStacks, lightSphereSlices);
	unsigned lightSphereIndexCount = (lightSphereStacks * lightSphereSlices + lightSphereSlices) * 6;

	unsigned screenSpaceQuadVertexArrayHandle = CreateScreenSpaceQuadMesh();

	ForwardRenderer* forwardRenderer = new ForwardRenderer(window);
	DeferredRenderer* deferredRenderer = new DeferredRenderer(window, screenSpaceQuadVertexArrayHandle, lightSphereVertexArrayHandle, lightSphereIndexCount);
	ForwardPlusRenderer* forwardPlusRenderer = new ForwardPlusRenderer(window);
	TiledDeferredRenderer* tiledDeferredRenderer = new TiledDeferredRenderer(window, screenSpaceQuadVertexArrayHandle);

	ImGuiRenderer* imGuiRenderer = new ImGuiRenderer(window);

	Renderer* renderers[4];
	renderers[0] = forwardRenderer;
	renderers[1] = deferredRenderer;
	renderers[2] = forwardPlusRenderer;
	renderers[3] = tiledDeferredRenderer;

	Mesh* bunnyMesh = LoadModel("media/models/bunny.obj");

	double totalFrameTime = 0.0;
	int frames = 0;

	while (!window->ShouldClose())
	{
		double start = glfwGetTime();

		window->PollEvents();
		imGuiRenderer->NewFrame();

		float distance = 2.0f + objectCountX + objectCountY;
		float x = distance * sinf(phi) * cosf(theta);
		float y = distance * cosf(phi);
		float z = distance * sinf(phi) * sinf(theta);
		glm::mat4 viewMatrix = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		renderers[currentRenderPipeline]->Render(projectionMatrix, viewMatrix, renderCommands, lights);
		frames++;

		{
			if (capturingFrameTimes || frameTimes.size() == MAX_FRAMES_TO_CAPTURE)
			{
				ImGui::SetNextWindowPosCenter(ImGuiCond_Once);
				ImGui::Begin("Capturing Frame Times...", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
				{
					float progress = static_cast<float>(frameTimes.size()) / MAX_FRAMES_TO_CAPTURE;
					ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
					ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				}
				ImGui::End();
			}
			else
			{
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Deliverable", nullptr, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
				{
					ImGui::SetWindowSize(ImVec2(static_cast<float>(window->GetWidth()) / 4.0f, static_cast<float>(window->GetHeight())));
					ImGui::Text(fpsString.c_str());
					ImGui::Text(frametimeString.c_str());

					if (ImGui::InputInt("Object Count X", &objectCountX) || ImGui::InputInt("Object Count Y", &objectCountY))
					{
						OnObjectCountChanged(bunnyMesh);
						OnLightCountChanged();

						for (int i = 0; i < 4; i++)
							renderers[i]->UpdateLightsShaderStorageBuffer(lights);
					}

					if (ImGui::InputInt("Light Count", &lightCount))
					{
						OnLightCountChanged();

						for (int i = 0; i < 4; i++)
							renderers[i]->UpdateLightsShaderStorageBuffer(lights);
					}

					if (ImGui::BeginCombo("Render Pipeline", renderPipelines[currentRenderPipeline]))
					{
						for (int n = 0; n < IM_ARRAYSIZE(renderPipelines); n++)
						{
							bool isSelected = (renderPipelines[currentRenderPipeline] == renderPipelines[n]);
							if (ImGui::Selectable(renderPipelines[n], isSelected))
								currentRenderPipeline = n;

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					if (ImGui::Button("Capture") && !capturingFrameTimes)
						capturingFrameTimes = true;
				}
				ImGui::End();
			}
		}

		imGuiRenderer->Render();
		window->SwapBuffers();

		double end = glfwGetTime();
		double passedTime = end - start;
		totalFrameTime += passedTime;

		if (capturingFrameTimes)
		{
			if (frameTimes.size() < MAX_FRAMES_TO_CAPTURE)
			{
				frameTimes.push_back(passedTime * 1000.0);
			}
			else
			{
				std::ofstream captureFile("capture.csv");
				for (size_t i = 0; i < frameTimes.size() - 1; i++)
					captureFile << frameTimes[i] << ",";
				captureFile << frameTimes.back();
				captureFile.close();

				frameTimes.clear();
				capturingFrameTimes = false;
			}
		}

		if (totalFrameTime >= 1.0)
		{
			fpsString = "FPS: " + std::to_string(frames);
			frametimeString = "Frame Time: " + std::to_string((totalFrameTime / frames) * 1000.0);
			totalFrameTime = 0.0;
			frames = 0;
		}
	}

	glDeleteVertexArrays(1, &lightSphereVertexArrayHandle);
	glDeleteVertexArrays(1, &screenSpaceQuadVertexArrayHandle);
	glDeleteVertexArrays(1, &bunnyMesh->vertexArrayHandle);
	delete bunnyMesh;

	delete forwardRenderer;
	delete deferredRenderer;
	delete forwardPlusRenderer;
	delete tiledDeferredRenderer;

	delete window;

	return 0;
}