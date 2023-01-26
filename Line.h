#pragma once

// 図形の描画
#include "Shape.h"

//
// 線による描画
//
class Line
  : public Shape
{
public:

  // コンストラクタ
  //   size: 頂点の位置の次元
  //   vertexcount: 頂点の数
  //   vertex: 頂点属性を格納した配列
  Line(GLint size, GLsizei vertexcount, const Object::Vertex* vertex)
    : Shape(size, vertexcount, vertex)
  {
  }

  // 描画の実行
  virtual void execute() const
  {
    // 線で描画する
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArrays(GL_PATCHES, 0, vertexcount);
  }
};

