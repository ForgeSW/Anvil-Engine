#include "AEngine.h"
#include "IGame.h"
#include "resource.h"
#include <fstream>
#include <iostream>

AEngine* AEngine::s_Instance = nullptr;

/**
 * Constructor for AEngine class
 * Initializes the engine components including window, OpenGL context, physics world,
 * shader, resource manager, and loads the game DLL
 */
AEngine::AEngine()
{
    // Store the instance of the engine
    s_Instance = this;
    // Initialize GLFW and create a window
    glfwInit();
    m_window = glfwCreateWindow(1280, 720, "Anvil Engine", NULL, NULL);
    glfwMakeContextCurrent(m_window);
    // Load OpenGL functions using GLAD
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Initialize physics world, shader, and resource manager
    m_physicsWorld    = new AnvilPhysics();
    m_mainShader      = new AShader(IDR_BASE_VERT, IDR_BASE_FRAG);
    m_resourceManager = new AResourceManager();

    // Load Game DLL dynamically
    m_gameLib = LoadLibraryA("Game.dll");
    if (m_gameLib)
    {
        // Define function pointer type for CreateGame function
        typedef IGame* (*CreateGameFn)();
        // Get the CreateGame function from the DLL
        CreateGameFn createGame = (CreateGameFn) GetProcAddress(m_gameLib, "CreateGame");
        if (createGame)
        {
            // Create game instance and initialize it
            m_game = createGame();
            m_game->OnInit(this);
        }
    }
}

/**
 * Loads a map file into the engine
 * @param mapName Name of the map file to load (without extension)
 */
void AEngine::LoadMap(const char* mapName)
{
    // Construct the full file path by adding the .absp extension
    std::string   path = std::string(mapName) + ".absp";
    std::ifstream is(path, std::ios::binary);
    if (!is)
    {
        std::cout << "Engine Error: Could not find " << path << std::endl;
        return;
    }
    for (GLuint tex : m_worldTextures)
        glDeleteTextures(1, &tex);
    m_worldTextures.clear();
    if (m_worldVAO)
    {
        glDeleteVertexArrays(1, &m_worldVAO);
        glDeleteBuffers(1, &m_worldVBO);
        glDeleteBuffers(1, &m_worldEBO);
    }

    ABSPHeader h;
    is.read((char*) &h, sizeof(h));

    m_worldVerts.resize(h.numVertices);
    m_worldFaces.resize(h.numFaces);
    is.read((char*) m_worldVerts.data(), h.numVertices * sizeof(AVertex));
    is.read((char*) m_worldFaces.data(), h.numFaces * sizeof(AFace));

    if (h.numEntities > 0)
    {
        std::vector<ABspEntity> entities(h.numEntities);
        is.read((char*) entities.data(), h.numEntities * sizeof(ABspEntity));

        for (const auto& ent : entities)
        {
            m_physicsWorld->AddTrigger(ent.position, ent.size, ent.name);
            std::cout << "Engine: Registered Trigger Entity -> " << ent.name << std::endl;
        }
    }
    if (h.numPlanes > 0)
    {
        std::vector<APlane> mapPlanes(h.numPlanes);
        is.read((char*) mapPlanes.data(), h.numPlanes * sizeof(APlane));

        m_physicsWorld->SetWorldPlanes(mapPlanes);
    }
    is.seekg(sizeof(ABSPHeader) + (h.numVertices * sizeof(AVertex)) + (h.numFaces * sizeof(AFace)) +
                 (h.numEntities * sizeof(ABspEntity)) + (h.numPlanes * sizeof(APlane)) +
                 (h.numBrushes * sizeof(ABSPBrush)),
             std::ios::beg);
    if (h.numTextures > 0)
    {
        std::vector<ATextureEntry> entries(h.numTextures);
        is.read((char*) entries.data(), h.numTextures * sizeof(ATextureEntry));

        for (const auto& te : entries)
        {
            std::vector<uint8_t> pixelData(te.dataSize);
            is.read((char*) pixelData.data(), te.dataSize);

            GLuint texID;
            glGenTextures(1, &texID);
            glBindTexture(GL_TEXTURE_2D, texID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, te.width, te.height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, pixelData.data());
            glGenerateMipmap(GL_TEXTURE_2D);

            m_worldTextures.push_back(texID);
        }
    }
    is.close();
    

    std::vector<uint32_t> indices;
    for (const auto& f : m_worldFaces)
    {
        for (uint32_t i = 1; i < f.numVertices - 1; i++)
        {
            indices.push_back(f.firstVertex);
            indices.push_back(f.firstVertex + i);
            indices.push_back(f.firstVertex + i + 1);
        }
    }
    m_worldIndexCount = (uint32_t) indices.size();

    m_physicsWorld->SetWorldData(m_worldVerts, m_worldFaces);

    glGenVertexArrays(1, &m_worldVAO);
    glGenBuffers(1, &m_worldVBO);
    glGenBuffers(1, &m_worldEBO);

    glBindVertexArray(m_worldVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_worldVBO);
    glBufferData(GL_ARRAY_BUFFER, m_worldVerts.size() * sizeof(AVertex), m_worldVerts.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_worldEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(),
                 GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AVertex), (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(AVertex), (void*) offsetof(AVertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(AVertex),
                          (void*) offsetof(AVertex, normal));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    std::cout << "Engine: Loaded " << path << " (" << m_worldIndexCount / 3 << " triangles, "
              << h.numEntities << " entities)" << std::endl;
}
/**
 * Creates a new entity and adds it to the engine's entity list
 * @param name The name to assign to the new entity
 * @return Pointer to the newly created entity
 */
AEntity* AEngine::CreateEntity(std::string name)
{
    // Create a new entity instance
    AEntity* ent = new AEntity();
    // Set the entity's name
    ent->name    = name;
    // Add the entity to the engine's entity list
    m_entities.push_back(ent);
    // Return the newly created entity
    return ent;
}

/**
 * Main game loop for the engine
 * Handles updating game state, physics, and rendering
 */
void AEngine::Run()
{
    // Main loop continues as long as the window is not closed
    while (!glfwWindowShouldClose(m_window))
    {
        // Calculate time delta for frame-independent movement
        float currentFrame = (float) glfwGetTime();
        m_deltaTime        = currentFrame - m_lastFrameTime;
        m_lastFrameTime    = currentFrame;

        // Update game state if a game instance exists
        if (m_game)
            m_game->OnUpdate(m_deltaTime);

        // Update all entities in the scene
        for (auto* e : m_entities)
            e->Update(m_deltaTime);

        // Update physics simulation
        m_physicsWorld->Update(m_deltaTime);

        // Rendering section
        // Clear the screen with a dark gray color
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render game objects if a game instance exists
        if (m_game)
        {
            // Use the main shader program
            m_mainShader->Use();

            // Set up projection matrix (perspective)
            glm::mat4 projection =
                glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 5000.0f);
            // Get view matrix from the game instance
            glm::mat4 view = m_game->GetViewMatrix();

            // Pass projection and view matrices to the shader
            glUniformMatrix4fv(glGetUniformLocation(m_mainShader->GetID(), "projection"), 1,
                               GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(m_mainShader->GetID(), "view"), 1, GL_FALSE,
                               glm::value_ptr(view));

            // Render World
            if (m_worldVAO)
            {
                m_mainShader->Use();
                glBindVertexArray(m_worldVAO);

                uint32_t indexOffset = 0;
                for (const auto& f : m_worldFaces)
                {
                    uint32_t indicesForFace = (f.numVertices - 2) * 3;
                    if (f.textureID < m_worldTextures.size())
                    {
                        glBindTexture(GL_TEXTURE_2D, m_worldTextures[f.textureID]);
                    }
                    else
                    {
                        glBindTexture(GL_TEXTURE_2D, 0); // Or a white texture
                    }
                    glDrawElements(GL_TRIANGLES, indicesForFace, GL_UNSIGNED_INT,
                                   (void*) (indexOffset * sizeof(uint32_t)));
                    indexOffset += indicesForFace;
                }
                // Draw the entire world in one single call
                glBindVertexArray(0);
            }

            // Render Entities
            for (auto* e : m_entities)
            {
                for (auto* c : e->GetComponents())
                    c->OnRender(m_mainShader);
            }
        }

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

AEngine::~AEngine()
{
    for (auto* e : m_entities)
        delete e;
    m_entities.clear();

    if (m_game)
    {
        m_game->OnShutdown();
        delete m_game;
    }

    if (m_worldVAO)
    {
        glDeleteVertexArrays(1, &m_worldVAO);
        glDeleteBuffers(1, &m_worldVBO);
        glDeleteBuffers(1, &m_worldEBO);
    }

    delete m_resourceManager;
    delete m_physicsWorld;
    delete m_mainShader;

    if (m_gameLib)
        FreeLibrary(m_gameLib);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}