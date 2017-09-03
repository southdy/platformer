#include "Base.h"
#include "Effect.h"
#include "LineBatch.h"
#include "Material.h"
#include "MeshBatch.h"

namespace gameplay
{
LineBatch::LineBatch() :
    _batch(nullptr)
{
}

LineBatch::~LineBatch()
{
    SAFE_DELETE(_batch);
}

LineBatch* LineBatch::create()
{
    // Vertex shader for drawing colored lines.
    const char* vs_str =
    {
        "uniform mat4 u_viewProjectionMatrix;\n"
        "attribute vec4 a_position;\n"
        "attribute vec4 a_color;\n"
        "varying vec4 v_color;\n"
        "void main(void) {\n"
        "    v_color = a_color;\n"
        "    gl_Position = u_viewProjectionMatrix * a_position;\n"
        "}"
    };

    // Fragment shader for drawing colored lines.
    const char* fs_str =
    {
    #ifdef OPENGL_ES
        "precision highp float;\n"
    #endif
        "varying vec4 v_color;\n"
        "void main(void) {\n"
        "   gl_FragColor = v_color;\n"
        "}"
    };

    Effect* effect = Effect::createFromSource(vs_str, fs_str);
    Material* material = Material::create(effect);
    GP_ASSERT(material && material->getStateBlock());
    material->getStateBlock()->setDepthTest(true);
    material->getStateBlock()->setDepthFunction(RenderState::DEPTH_LEQUAL);

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::COLOR, 4),
    };
    LineBatch * _lineBatch = new LineBatch();
    _lineBatch->_batch = MeshBatch::create(VertexFormat(elements, 2), Mesh::LINES, material, false, 4096, 4096);
    SAFE_RELEASE(material);
    SAFE_RELEASE(effect);
    return _lineBatch;
}


struct LineVertex
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
};

void LineBatch::addLine(const Vector3& from, const Vector3& to, const Vector4& fromColor, const Vector4& toColor)
{
    float const width = from.distance(to);
    float const halfWidth = width / 2;
    Rectangle bounds;
    bounds.x = ((from.x + to.x) / 2) - halfWidth;
    bounds.y = ((from.y + to.y) / 2) - halfWidth;
    bounds.width = width;
    bounds.height = width;
    if(!_viewport.intersects(bounds))
    {
        return;
    }
    GP_ASSERT(_batch);

    static LineVertex vertices[2];

    vertices[0].x = from.x;
    vertices[0].y = from.y;
    vertices[0].z = from.z;
    vertices[0].r = fromColor.x;
    vertices[0].g = fromColor.y;
    vertices[0].b = fromColor.z;
    vertices[0].a = fromColor.w;

    vertices[1].x = to.x;
    vertices[1].y = to.y;
    vertices[1].z = to.z;
    vertices[1].r = toColor.x;
    vertices[1].g = toColor.y;
    vertices[1].b = toColor.z;
    vertices[1].a = toColor.w;

    _batch->add(vertices, 2);
}

void LineBatch::start(const Matrix& viewProjection, const Rectangle& viewport)
{
    _viewport = viewport;
    _batch->start();
    _batch->getMaterial()->getParameter("u_viewProjectionMatrix")->setValue(viewProjection);
}

void LineBatch::finish()
{
    _batch->finish();
    _batch->draw();
}
}
