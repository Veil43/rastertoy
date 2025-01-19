#pragma once
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

// NOTE: The obj loader in this rasterizer does not support:
// - Non-convex polygons.
// - Polygons with holes.

// VERTEX THINGS ---------------------------------------------------
struct vertex2
{
    color4 color;
    vec2f point;
};

struct vertex3
{
    vec3f point;
    vec3f normal;
    color4 color;
};

inline std::ostream&
operator<<(std::ostream& o, const vertex3& v)
{
    o << "POINT: " << v.point << " | COLOR: " << v.color;
    return o;
}

/*
40 bytes
*/
struct Model3D
{
    // These form a vertex3
    vec3f *VertexPositions;
    vec3f *vertexNormals;
    color4 *VertexColors;

    // These form triangles
    int32 *vertexIndices;
    int32 *normalIndices;

    uint32 vn;  // vertex count
    uint32 in;  // index count
};

/*
328 bytes
*/
class Object3D
{
public:
    mat4x4 rotation;
    Model3D *ObjectModel;
    sphere BoundingSphere;
    vec3f position;
    uint32 ID;
    real32 scale;

    // should we really return a copy? yes?
    const sphere ObjectBoundingSphere() const
    {
        sphere bSphere = BoundingSphere;
        bSphere.center = bSphere.center * scale;
        bSphere.radius = bSphere.radius * scale;
        bSphere.center = bSphere.center * rotation;
        bSphere.center = bSphere.center + position;

        return bSphere;
    }

    const mat4x4 ObjectTransform() const
    {
        mat4x4 i = I_MATRIX_4X4;
        i *= scale;
        i *= rotation;
        i.r3 = {position.x, position.y, position.z, 1};

        return i;
    }

    const mat4x4 ObjectRotation() const
    {
        return rotation;
    }

    void RotateObjectX(real32 deg)
    {
        mat4x4 RotationX = get_x_rotation_mat(deg);
        rotation *= RotationX;
    }
    void RotateObjectY(real32 deg)
    {
        mat4x4 RotationY = get_y_rotation_mat(deg);
        rotation *= RotationY;
    }

    void RotateObjectZ(real32 deg)
    {
        mat4x4 RotationZ = get_z_rotation_mat(deg);
        rotation *= RotationZ;
    }
};

static Object3D *
CreateCube(const vec3f& position, real32 size = 1, uint32 ID = 0xFFFFFFFF)
{
    Model3D *Model = new Model3D;
    vec3f *VertexPositions = new vec3f[24]
    {
        { 0.5,  0.5, -0.5}, { 0.5, -0.5, -0.5}, {-0.5, -0.5, -0.5}, {-0.5,  0.5, -0.5}, //FRONT
        { 0.5,  0.5,  0.5}, { 0.5, -0.5,  0.5}, { 0.5, -0.5, -0.5}, { 0.5,  0.5, -0.5}, // RIGHT
        {-0.5,  0.5,  0.5}, {-0.5, -0.5,  0.5}, { 0.5, -0.5,  0.5}, { 0.5,  0.5,  0.5}, // BACK
        {-0.5,  0.5, -0.5}, {-0.5, -0.5, -0.5}, {-0.5, -0.5,  0.5}, {-0.5,  0.5,  0.5}, // LEFT
        { 0.5,  0.5,  0.5}, { 0.5,  0.5, -0.5}, {-0.5,  0.5, -0.5}, {-0.5,  0.5,  0.5}, // TOP
        {-0.5, -0.5,  0.5}, {-0.5, -0.5, -0.5}, { 0.5, -0.5, -0.5}, { 0.5, -0.5,  0.5}  // BOTTOM
    };

    color4 *VertexColors = new color4[24]
    {
        RED,    RED,    RED,    RED,
        GREEN,  GREEN,  GREEN,  GREEN,
        BLUE,   BLUE,   BLUE,   BLUE,
        YELLOW, YELLOW, YELLOW, YELLOW,
        PURPLE, PURPLE, PURPLE, PURPLE,
        CYAN,   CYAN,   CYAN,   CYAN
    };

    // Winding counter-clockwise
    int32 *Indices = new int32[36]
    { 
        0, 1, 2,    0, 2, 3,    // FRONT
        4, 5, 6,    4, 6, 7,    // RIGHT
        8, 9, 10,   8, 10, 11,  // BACK
        12, 13, 14, 12, 14, 15, // LEFT
        16, 17, 18, 16, 18, 19, // TOP
        20, 21, 22, 20, 22, 23  // BOTTOM
    };
    
    vec3f *vertexNormals = new vec3f[24]
    {
        -VECTOR_K3, -VECTOR_K3, -VECTOR_K3, -VECTOR_K3, // FRONT
         VECTOR_I3,  VECTOR_I3,  VECTOR_I3,  VECTOR_I3, // RIGHT
         VECTOR_K3,  VECTOR_K3,  VECTOR_K3,  VECTOR_K3, // BACK
        -VECTOR_I3, -VECTOR_I3, -VECTOR_I3, -VECTOR_I3, // LEFT
         VECTOR_J3,  VECTOR_J3,  VECTOR_J3,  VECTOR_J3, // TOP  
        -VECTOR_J3, -VECTOR_J3, -VECTOR_J3, -VECTOR_J3  // BOTTOM
    };

    int32 *normalIndices = new int32[36]
    { 
        0, 1, 2,    0, 2, 3,    // FRONT
        4, 5, 6,    4, 6, 7,    // RIGHT
        8, 9, 10,   8, 10, 11,  // BACK
        12, 13, 14, 12, 14, 15, // LEFT
        16, 17, 18, 16, 18, 19, // TOP
        20, 21, 22, 20, 22, 23  // BOTTOM
    };

    Model->VertexPositions = VertexPositions;
    Model->vertexNormals = vertexNormals;
    Model->VertexColors = VertexColors;
    Model->vertexIndices = Indices;
    Model->normalIndices = normalIndices;
    Model->vn = 8;
    Model->in = 36;

    Object3D *Cube = new Object3D;
    
    Cube->ObjectModel = Model;
    Cube->scale = size;
    Cube->position = position;
    Cube->rotation = I_MATRIX_4X4;

    vec3f average_center = {0, 0, 0};
    real32 radius = 0.0f;
    for (int i = 0; i < Cube->ObjectModel->vn; ++i)
    {
        average_center += Cube->ObjectModel->VertexPositions[i];
    }
    average_center /= Cube->ObjectModel->vn;

    for (int i = 0; i < Cube->ObjectModel->vn; ++i)
    {
        real32 square_length = length_squared(Cube->ObjectModel->VertexPositions[i] - average_center);
        radius = square_length > radius ? square_length : radius;
    }

    radius = static_cast<real32>(std::sqrt(radius)) * size;

    Cube->BoundingSphere = {average_center, radius};

    Cube->ID = ID;
    return Cube;
}

// Obj Parser ------------------------------------------------------------------------

Object3D *
LoadObjectFromOBJ(std::string name, const vec3f& position, real32 size)
{
    std::string filePath = "./data/" + name;
    std::ifstream objFile(filePath);

    if (!objFile.is_open())
    {
        objFile = std::ifstream("." + filePath);

        if (!objFile.is_open())
        {
            std::cerr << "Error: Could not open file: " << filePath << "!" << std::endl;
            return nullptr;
        }
    }

    std::string line;
    std::vector<vec3f> vertices;
    std::vector<vec3f> normals;
    std::vector<int32> vertexIndices;
    std::vector<int32> normalIndices;
    while (std::getline(objFile, line))
    {
        std::istringstream stream(line);
        std::string word;
        stream >> word;

        if (word == "v")
        {
            try
            {
                float values[3];
                for (int i = 0; i < 3; ++i)
                {
                    stream >> word;
                    values[i] = std::stof(word);
                }
                vec3f vec = {values[0], values[1], values[2]};
                vertices.push_back(vec);
            }
            catch(const std::invalid_argument& e)
            {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            }
            catch(const std::out_of_range& e)
            {
                std::cerr << "Out of range: " << e.what() << std::endl;
            }
        }
        else if (word == "vn")
        {
            try
            {
                float values[3];
                for (int i = 0; i < 3; ++i)
                {
                    stream >> word;
                    values[i] = std::stof(word);
                }
                vec3f vec = {values[0], values[1], values[2]};
                normals.push_back(vec);
            }
            catch(const std::invalid_argument& e)
            {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            }
            catch(const std::out_of_range& e)
            {
                std::cerr << "Out of range: " << e.what() << std::endl;
            }
        }
        else if (word == "f")
        {
            std::vector<int32> faceVertexIndices;
            std::vector<int32> faceNormalIndices;
            try
            {
                while(stream >> word)
                {
                    std::vector<std::string> indicesTemp;
                    std::stringstream ss(word);
                    std::string index;
                    while(std::getline(ss, index, '/'))
                    {
                        indicesTemp.push_back(index);
                    }

                    // parse the vertex index
                    if (!indicesTemp[0].empty())
                    {
                        faceVertexIndices.push_back(std::stoi(indicesTemp[0]));
                    }

                    // parse normals
                    if (indicesTemp.size() > 2 && !indicesTemp[2].empty())
                    {
                        faceNormalIndices.push_back(std::stoi(indicesTemp[2]));
                    }
                }

                // plit quads into two triangels 1, 2, 3 and 1, 3, 4.
                if (faceVertexIndices.size() == 3)
                {
                    vertexIndices.insert(vertexIndices.end(), faceVertexIndices.begin(), faceVertexIndices.end());
                    normalIndices.insert(normalIndices.end(), faceNormalIndices.begin(),faceNormalIndices.end());
                }
                else if (faceVertexIndices.size() == 4)
                {
                    vertexIndices.push_back(faceVertexIndices[0]);
                    vertexIndices.push_back(faceVertexIndices[1]);
                    vertexIndices.push_back(faceVertexIndices[2]);
                    vertexIndices.push_back(faceVertexIndices[0]);
                    vertexIndices.push_back(faceVertexIndices[2]);
                    vertexIndices.push_back(faceVertexIndices[3]);

                    if (faceNormalIndices.size() == 4)
                    {
                        normalIndices.push_back(faceNormalIndices[0]);
                        normalIndices.push_back(faceNormalIndices[1]);
                        normalIndices.push_back(faceNormalIndices[2]);
                        normalIndices.push_back(faceNormalIndices[0]);
                        normalIndices.push_back(faceNormalIndices[2]);
                        normalIndices.push_back(faceNormalIndices[3]);
                    }
                }
                else if (faceVertexIndices.size() > 4)
                {
                    int32 pivotVertex = faceVertexIndices[0];
                    for (size_t i = 1; i < faceVertexIndices.size() - 1; ++i)
                    {
                        vertexIndices.push_back(pivotVertex);
                        vertexIndices.push_back(faceVertexIndices[i]);
                        vertexIndices.push_back(faceVertexIndices[i + 1]);
                    }
                    
                    if (faceNormalIndices.size() > 4)
                    {
                        int32 pivotNormal = faceNormalIndices[0];
                        for (size_t i = 1; i < faceNormalIndices.size() - 1; ++i)
                        {
                            normalIndices.push_back(pivotNormal);
                            normalIndices.push_back(faceNormalIndices[i]);
                            normalIndices.push_back(faceNormalIndices[i+1]);
                        }
                    }

                    std::cerr << "WARNING: render toy only supports faces of 4 or less vertices. Non-convex polygons will have undefined behaviour!\n";
                }
            }
            catch(const std::invalid_argument& e)
            {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            }
            catch(const std::out_of_range& e)
            {
                std::cerr << "Out of range: " << e.what() << std::endl;
            }
        }
        else
        {
            continue;
        }
    }

    if (normalIndices.empty())
    {
        normalIndices = vertexIndices;
    }

    if (normals.empty() || vertices.size() != normals.size())
    {
        std::cerr << "WARNING: Normals not provided. rastertoy will attempt generating normals\n";
        normals.resize(vertices.size());
        for (size_t i = 0; i < normalIndices.size() / 3; ++i)
        {
            int in0 = normalIndices[i * 3] - 1;
            int in1 = normalIndices[i * 3 + 1] - 1;
            int in2 = normalIndices[i * 3 + 2] - 1;

            vec3f v0 = vertices[in0];
            vec3f v1 = vertices[in1];
            vec3f v2 = vertices[in2];

            vec3f n = (cross(v1 - v0, v2 - v0));
            normals[in0] += n; // n0
            normals[in1] += n; // n1
            normals[in2] += n; // n2
        }
    }

    assert(vertexIndices.size() == normalIndices.size());
    assert(vertices.size() == normals.size());

    Model3D *objectModel = new Model3D;

    vec3f *vertexPositions = new vec3f[vertices.size()];
    vec3f *vertexNormals = new vec3f[normals.size()];
    color4 *vertexColors = new color4[vertices.size()];
    int32 *vertexIndicesTemp = new int[vertexIndices.size()];
    int32 *normalIndicesTemp = new int[normalIndices.size()];

    Object3D *newObject = new Object3D;
    newObject->rotation = I_MATRIX_4X4;
    newObject->ObjectModel = objectModel;
    vec3f origin = {0.f,0.f,0.f};
    for (const auto& v : vertices)
    {
        origin += v;
    }
    if (!vertices.empty())
    {
        origin /= vertices.size();
    }

    real32 radius = 0.f;
    for (const auto& v : vertices)
    {
        real32 square_length = length_squared(v - origin);
        radius = square_length > radius ? square_length : radius;
    }
    radius = std::sqrt(radius);

    newObject->BoundingSphere = sphere{origin, radius};
    newObject->position = position;
    newObject->ID = 0;
    newObject->scale = size;

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertexPositions[i] = vertices[i] / radius;
    }
    objectModel->VertexPositions = vertexPositions;

    for (size_t i = 0; i < normals.size(); ++i)
    {
        vertexNormals[i] = normalize(normals[i]);
    }
    objectModel->vertexNormals = vertexNormals;

    std::fill(vertexColors, vertexColors + vertices.size(), DEFAULT);
    objectModel->VertexColors = vertexColors;

    for (size_t i = 0; i < vertexIndices.size(); ++i)
    {
        vertexIndicesTemp[i] = vertexIndices[i] - 1;
    }
    objectModel->vertexIndices = vertexIndicesTemp;

    for (size_t i = 0; i < normalIndices.size(); ++i)
    {
        normalIndicesTemp[i] = normalIndices[i] - 1;
    }
    objectModel->normalIndices = normalIndicesTemp;

    objectModel->vn = vertices.size();
    objectModel->in = vertexIndices.size();

    std::cout << name << " has been loaded\n";
    std::cout << "Vertices: " << vertices.size() << "\n";
    std::cout << "Normals: " << normals.size() << "\n";
    std::cout << "Faces :" << vertexIndices.size() / 3 << std::endl;
    objFile.close();
    return newObject;
}

void
DestroyModel(Model3D *Model)
{
    delete[] Model->VertexPositions;
    delete[] Model->vertexNormals;
    delete[] Model->VertexColors;
    delete[] Model->vertexIndices;
    delete   Model;

    Model = nullptr;
}

void
DestroyObject3D(Object3D *WorldObject)
{
    DestroyModel(WorldObject->ObjectModel);
    delete WorldObject;

    WorldObject = nullptr;
}
