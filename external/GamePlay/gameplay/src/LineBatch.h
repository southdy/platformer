#ifndef LINEBATCH_H_
#define LINEBATCH_H_

#include "Rectangle.h"

namespace gameplay
{
class MeshBatch;
class Vector3;
class Vector4;

class LineBatch
{
public:
    ~LineBatch();
    static LineBatch* create();
    void addLine(const Vector3& from, const Vector3& to, const Vector4& fromColor, const Vector4& toColor);
    void start(const Matrix& viewProjection, const Rectangle& viewport);
    void finish();
private:
    LineBatch();
    MeshBatch* _batch;
    Rectangle _viewport;
};
}

#endif
