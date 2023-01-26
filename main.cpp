#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Window.h"
#include "Matrix.h"
#include "Vector.h"
#include "Shape.h"
#include "ShapeIndex.h"
#include "SolidShapeIndex.h"
#include "SolidShape.h"
#include "Uniform.h"
#include "Material.h"
#include "Line.h"
#include "cyCore.h"
#include "cyVector.h"

// シェーダオブジェクトのコンパイル結果を表示する
//   shader: シェーダオブジェクト名
//   str: コンパイルエラーが発生した場所を示す文字列
GLboolean printShaderInfoLog(GLuint shader, const char *str)
{
  // コンパイル結果を取得する
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) std::cerr << "Compile Error in " << str << std::endl;

  // シェーダのコンパイル時のログの長さを取得する
  GLsizei bufSize;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);

  if (bufSize > 1)
  {
    // シェーダのコンパイル時のログの内容を取得する
    std::vector<GLchar> infoLog(bufSize);
    GLsizei length;
    glGetShaderInfoLog(shader, bufSize, &length, &infoLog[0]);
    std::cerr << &infoLog[0] << std::endl;
  }

  return static_cast<GLboolean>(status);
}

// プログラムオブジェクトのリンク結果を表示する
//   program: プログラムオブジェクト名
GLboolean printProgramInfoLog(GLuint program)
{
  // リンク結果を取得する
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) std::cerr << "Link Error." << std::endl;

  // シェーダのリンク時のログの長さを取得する
  GLsizei bufSize;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);

  if (bufSize > 1)
  {
    // シェーダのリンク時のログの内容を取得する
    std::vector<GLchar> infoLog(bufSize);
    GLsizei length;
    glGetProgramInfoLog(program, bufSize, &length, &infoLog[0]);
    std::cerr << &infoLog[0] << std::endl;
  }

  return static_cast<GLboolean>(status);
}

// プログラムオブジェクトを作成する
//   vsrc: バーテックスシェーダのソースプログラムの文字列
//   fsrc: フラグメントシェーダのソースプログラムの文字列
GLuint createProgram(const char *vsrc, const char* csrc, const char* esrc, const char* gsrc, const char *fsrc)
{
  // 空のプログラムオブジェクトを作成する
  const GLuint program(glCreateProgram());

  if (vsrc != NULL)
  {
    // バーテックスシェーダのシェーダオブジェクトを作成する
    const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
    glShaderSource(vobj, 1, &vsrc, NULL);
    glCompileShader(vobj);

    // バーテックスシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
    if (printShaderInfoLog(vobj, "vertex shader"))
      glAttachShader(program, vobj);
    glDeleteShader(vobj);
  }

  if (csrc != NULL)
  {
    // テッセレーション制御シェーダのシェーダオブジェクトを作成する
    const GLuint cobj(glCreateShader(GL_TESS_CONTROL_SHADER));
    glShaderSource(cobj, 1, &csrc, NULL);
    glCompileShader(cobj);

    // テッセレーション制御シェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
    if (printShaderInfoLog(cobj, "tessellation control shader"))
      glAttachShader(program, cobj);
    glDeleteShader(cobj);
  }

  if (esrc != NULL)
  {
    // テッセレーション評価シェーダのシェーダオブジェクトを作成する
    const GLuint eobj(glCreateShader(GL_TESS_EVALUATION_SHADER));
    glShaderSource(eobj, 1, &esrc, NULL);
    glCompileShader(eobj);

    // テッセレーション評価シェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
    if (printShaderInfoLog(eobj, "tessellation evaluation shader"))
      glAttachShader(program, eobj);
    glDeleteShader(eobj);
  }

  /*
  if (gsrc != NULL)
  {
    // ジオメトリシェーダのシェーダオブジェクトを作成する
    const GLuint gobj(glCreateShader(GL_GEOMETRY_SHADER));
    glShaderSource(gobj, 1, &gsrc, NULL);
    glCompileShader(gobj);

    // ジオメトリシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
    if (printShaderInfoLog(gobj, "geometry shader"))
      glAttachShader(program, gobj);
    glDeleteShader(gobj);
  }
  */
  if (fsrc != NULL)
  {
    // フラグメントシェーダのシェーダオブジェクトを作成する
    const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
    glShaderSource(fobj, 1, &fsrc, NULL);
    glCompileShader(fobj);

    // フラグメントシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
    if (printShaderInfoLog(fobj, "fragment shader"))
      glAttachShader(program, fobj);
    glDeleteShader(fobj);
  }

  // プログラムオブジェクトをリンクする
  glBindAttribLocation(program, 0, "position");
  glBindAttribLocation(program, 1, "normal");
  glBindFragDataLocation(program, 0, "fragment");
  glLinkProgram(program);

  // 作成したプログラムオブジェクトを返す
  if (printProgramInfoLog(program))
    return program;

  // プログラムオブジェクトが作成できなければ 0 を返す
  glDeleteProgram(program);
  return 0;
}

// シェーダのソースファイルを読み込む
//   name: シェーダのソースファイル名
//   buffer: 読み込んだソースファイルのテキスト
bool readShaderSource(const char *name, std::vector<GLchar> &buffer)
{
  // ファイル名が NULL だった
  if (name == NULL) return false;

  // ソースファイルを開く
  std::ifstream file(name, std::ios::binary);
  if (file.fail())
  {
    // 開けなかった
    std::cerr << "Error: Can't open source file: " << name << std::endl;
    return false;
  }

  // ファイルの末尾に移動し現在位置（＝ファイルサイズ）を得る
  file.seekg(0L, std::ios::end);
  GLsizei length = static_cast<GLsizei>(file.tellg());

  // ファイルサイズのメモリを確保
  buffer.resize(length + 1);

  // ファイルを先頭から読み込む
  file.seekg(0L, std::ios::beg);
  file.read(buffer.data(), length);
  buffer[length] = '\0';

  if (file.fail())
  {
    // うまく読み込めなかった
    std::cerr << "Error: Could not read souce file: " << name << std::endl;
    file.close();
    return false;
  }

  // 読み込み成功
  file.close();
  return true;
}

// シェーダのソースファイルを読み込んでプログラムオブジェクトを作成する
//   vert: バーテックスシェーダのソースファイル名
//   frag: フラグメントシェーダのソースファイル名
GLuint loadProgram(const char *vert, const char* tcs, const char* tes, const char* gs, const char *frag)
{
  // シェーダのソースファイルを読み込む
  std::vector<GLchar> vsrc;
  const bool vstat(readShaderSource(vert, vsrc));
  std::vector<GLchar> csrc;
  const bool cstat(readShaderSource(tcs, csrc));
  std::vector<GLchar> esrc;
  const bool estat(readShaderSource(tes, esrc));
  std::vector<GLchar> gsrc;
  const bool gstat(readShaderSource(gs, gsrc));
  std::vector<GLchar> fsrc;
  const bool fstat(readShaderSource(frag, fsrc));

  // プログラムオブジェクトを作成する
  return vstat && fstat ? createProgram(vsrc.data(), csrc.data(), esrc.data(), gsrc.data(), fsrc.data()) : 0;
}

/*
void load_obj(const char* filename, int& LineLength, std::vector<Object::Vertex>& vertices) { //,Object::Vertex& normals, std::vector<GLushort>& elements) {
  std::ifstream in(filename, std::ios::in);
  if (!in) { std::cerr << "Cannot open " << filename << std::endl; exit(1); }
  int cnt = 0;
  std::string line;
  while (std::getline(in, line)) {
    if (line.substr(0, 2) == "v ") {
      std::istringstream s(line.substr(2));
      GLfloat readV[3];
      s >> readV[0]; s >> readV[1]; s >> readV[2];
      Object::Vertex v = { readV[0],readV[1],readV[2] };
      vertices.push_back(v);
      cnt++;
    }
    /*   頂点の座標のみを考慮している
    else if (line.substr(0, 2) == "f ") {
      std::istringstream s(line.substr(2));
      GLushort a, b, c;
      s >> a; s >> b; s >> c;
      a--; b--; c--;
      elements.push_back(a); elements.push_back(b); elements.push_back(c);
    }
    //else if (line[0] == '#') { /* ignoring this line //}
    //else { /* ignoring this line  }
  }
  /*
  normals.resize(mesh->vertices.size(), glm::vec3(0.0, 0.0, 0.0));
  for (int i = 0; i < elements.size(); i += 3) {
    GLushort ia = elements[i];
    GLushort ib = elements[i + 1];
    GLushort ic = elements[i + 2];
    glm::vec3 normal = glm::normalize(glm::cross(
      glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
      glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
    normals[ia] = normals[ib] = normals[ic] = normal;
  }
  LineLength = cnt;
}
*/

struct BCCHeader
{
  char sign[3];
  unsigned char byteCount;
  char curveType[2];
  char dimensions;
  char upDimension;
  uint64_t curveCount;
  uint64_t totalControlPointCount;
  char fileInfo[40];
};

int main()
{
  // GLFW を初期化する
  if (glfwInit() == GL_FALSE)
  {
    // 初期化に失敗した
    std::cerr << "Can't initialize GLFW" << std::endl;
    return 1;
  }

  // プログラム終了時の処理を登録する
  atexit(glfwTerminate);

  // OpenGL Version 4.6 Core Profile を選択する
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // ウィンドウを作成する
  Window window;

  // 背景色を指定する
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // 背面カリングを有効にする
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  // デプスバッファを有効にする
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  // プログラムオブジェクトを作成する
  const GLuint program(loadProgram("realtime.vert", "realtime.tcs", "realtime.tes", "realtime.gs", "realtime.frag"));

  // uniform 変数の場所を取得する
  const GLint modelviewLoc(glGetUniformLocation(program, "modelview"));
  const GLint projectionLoc(glGetUniformLocation(program, "projection"));
  const GLint normalMatrixLoc(glGetUniformLocation(program, "normalMatrix"));
  const GLint LposLoc(glGetUniformLocation(program, "Lpos"));
  const GLint LambLoc(glGetUniformLocation(program, "Lamb"));
  const GLint LdiffLoc(glGetUniformLocation(program, "Ldiff"));
  const GLint LspecLoc(glGetUniformLocation(program, "Lspec"));
  const GLint NumSegments(glGetUniformLocation(program, "NumSegments"));
  const GLint NumStrips(glGetUniformLocation(program, "NumStrips"));
  const GLint LineColor(glGetUniformLocation(program, "LineColor"));
  const GLint Para_alpha(glGetUniformLocation(program, "Para_alpha"));
  const GLint Para_Rmin(glGetUniformLocation(program, "Para_Rmin"));
  const GLint Para_s(glGetUniformLocation(program, "Para_s"));
  const GLint Para_eX(glGetUniformLocation(program, "Para_eX"));
  const GLint Para_eY(glGetUniformLocation(program, "Para_eY"));
  const GLint Para_Rply(glGetUniformLocation(program, "Para_Rply"));
  const GLint Para_aply(glGetUniformLocation(program, "Para_aply"));
  const GLint Para_m(glGetUniformLocation(program, "Para_m"));
  const GLint Para_Rholoop(glGetUniformLocation(program, "Para_Rholoop"));
  const GLint Para_Rhohair(glGetUniformLocation(program, "Para_Rhohair"));
  const GLint Para_loopmax(glGetUniformLocation(program, "Para_loopmax"));
  const GLint Para_hairmin(glGetUniformLocation(program, "Para_hairmin"));
  const GLint Para_Rhairspan(glGetUniformLocation(program, "Para_Rhairspan"));
  const GLint Para_zhairspan(glGetUniformLocation(program, "Para_zhairspan"));
  const GLint Para_thairspan(glGetUniformLocation(program, "Para_thairspan"));
  const GLint Para_Ri(glGetUniformLocation(program, "Para_Ri"));
  const GLint Para_ti(glGetUniformLocation(program, "Para_ti"));

  // uniform block の場所を取得する
  const GLint materialLoc(glGetUniformBlockIndex(program, "Material"));

  // uniform block の場所を 0 番の結合ポイントに結びつける
  glUniformBlockBinding(program, materialLoc, 0);

  
  
  // 頂点属性を作る
  std::vector<Object::Vertex> line;

  // インデックスを作る
  std::vector<GLuint> solidSphereIndex;

  //頂点情報
  //BCC pointer error
  /*
  BCCHeader header;
  FILE* pFile = fopen("filename.bcc", "rb");
  fread(&header, sizeof(header), 1, pFile);
  if (header.sign[0] != 'B') return -1; // Invalid file signature
  if (header.sign[1] != 'C') return -1; // Invalid file signature
  if (header.sign[2] != 'C') return -1; // Invalid file signature
  if (header.byteCount != 0x44) return -1; // Only supporting 4-byte integers and floats
  if (header.curveType[0] != 'C') return -1; // Not a Catmull-Rom curve
  if (header.curveType[1] != '0') return -1; // Not uniform parameterization
  if (header.dimensions != 3) return -1; // Only curves in 3D

  std::vector<cyVec3f> controlPoints(header.totalControlPointCount); //制御点
  std::vector<int> firstControlPoint(header.curveCount + 1); //はじめの制御点のインデックス？
  std::vector<char> isCurveLoop(header.curveCount);//
  float* cp = controlPoints.data();//制御点の座標
  int prevCP = 0;//最初の制御点を示す
  for (uint64_t i = 0; i < header.curveCount; i++) {
    int curveControlPointCount;
    fread(&curveControlPointCount, sizeof(int), 1, pFile); //curveControlPointCountに読み込む
    isCurveLoop[i] = curveControlPointCount < 0; //0以下のとき代入
    if (curveControlPointCount < 0) curveControlPointCount = -curveControlPointCount;//0以下のとき符号反転
    fread(cp, sizeof(float), curveControlPointCount, pFile);//読み込みデータ数curveControlPointCount
    cp += curveControlPointCount;//curveControlPointCountずつ増える
    firstControlPoint[i] = prevCP;//最初の制御点の位置
    prevCP += curveControlPointCount;//最初の制御点はcurveControlPointCountずつ移動する
  }
  firstControlPoint[header.curveCount] = prevCP;//最後の代入も忘れずに
  */

  /* .obj
  int LineLength = 0; //頂点数
  std::vector<Object::Vertex> LineVertex0; //仮頂点配列
  load_obj("dress1.obj",LineLength, LineVertex0); //頂点配列オブジェクトの読み込み
  Object::Vertex* LineVertex = &LineVertex0[0]; //頂点配列に変換
  std::cerr << LineVertex[34].position[0] << LineVertex[34].position[1] << LineVertex[34].position[2] << std::endl;
  */
  Object::Vertex LineVertex[] =
  {
    
    {-2.0f,0.0f,0.0f},
    {-1.5f,0.0f,0.0f},
    {-1.0f,0.0f,0.0f},
    {-0.5f,0.0f,0.0f},
    {-1.5f,0.0f,0.0f},
    {-1.0f,0.0f,0.0f},
    {-0.5f,0.0f,0.0f},
    {0.0f,0.0f,0.0f},
    {-1.0f,0.0f,0.0f},
    {-0.5f,0.0f,0.0f},
    {0.0f,0.0f,0.0f},
    {0.5f,0.0f,0.0f},
    {-0.5f,0.0f,0.0f},
    {0.0f,0.0f,0.0f},
    {0.5f,0.0f,0.0f},
    {1.0f,0.0f,0.0f},
    {0.0f,0.0f,0.0f},
    {0.5f,0.0f,0.0f},
    {1.0f,0.0f,0.0f},
    {1.5f,0.0f,0.0f},
    {0.5f,0.0f,0.0f},
    {1.0f,0.0f,0.0f},
    {1.5f,0.0f,0.0f},
    {2.0f,0.0f,0.0f}
    
    /*
    {-1.0,0.0,0.0},
    {-0.5,0.0,0.0},
    {0.5,0.0,0.0},
    {1.0,0.0,0.0}
    */
  };


  // 図形データを作成する
  std::unique_ptr<const Shape> shape(new Line(3, 24, LineVertex)); //制御点の数を変えた時ここも変更

  // 光源データ
  static constexpr int Lcount(2);
  static constexpr Vector Lpos[] = { 0.0f, 0.0f, 10.0f, 1.0f, 8.0f, 0.0f, 0.0f, 1.0f };
  static constexpr GLfloat Lamb[] = { 0.2f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f };
  static constexpr GLfloat Ldiff[] = { 1.0f, 0.5f, 0.5f, 0.9f, 0.9f, 0.9f };
  static constexpr GLfloat Lspec[] = { 1.0f, 0.5f, 0.5f, 0.9f, 0.9f, 0.9f };

  // 色データ
  static constexpr Material color[] =
  {
    //      Kamb               Kdiff              Kspec        Kshi
    { 0.6f, 0.6f, 0.2f,  0.6f, 0.6f, 0.2f,  0.3f, 0.3f, 0.3f,  30.0f },
    { 0.1f, 0.1f, 0.5f,  0.1f, 0.1f, 0.5f,  0.4f, 0.4f, 0.4f,  60.0f }
  };
  const Uniform<Material> material(color, 2);

  // タイマーを 0 にセット
  glfwSetTime(0.0);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // シェーダプログラムの使用開始
    glUseProgram(program);
    
    // 透視投影変換行列を求める
    const GLfloat *const size(window.getSize());
    const GLfloat fovy(window.getScale() * 0.01f);
    const GLfloat aspect(size[0] / size[1]);
    const Matrix projection(Matrix::perspective(fovy, aspect, 1.0f, 10.0f));

    // モデル変換行列を求める
    const GLfloat *const location(window.getLocation());
    //const Matrix r(Matrix::rotate(static_cast<GLfloat>(glfwGetTime()), 0.0f, 1.0f, 0.0f)); 回転に使う
    const Matrix model(Matrix::translate(location[0], location[1], 0.0f));

    // ビュー変換行列を求める
    const Matrix view(Matrix::lookat(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));

    // 法線ベクトルの変換行列の格納先
    //GLfloat normalMatrix[9];

    // モデルビュー変換行列を求める
    const Matrix modelview(view * model);

    // 法線ベクトルの変換行列を求める
    //modelview.getNormalMatrix(normalMatrix);
    
    // uniform 変数に値を設定する
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data());
    glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, modelview.data());
    //glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, normalMatrix);
    for (int i = 0; i < Lcount; ++i)
      glUniform4fv(LposLoc + i, 1, (view * Lpos[i]).data());
    glUniform3fv(LambLoc, Lcount, Lamb);
    glUniform3fv(LdiffLoc, Lcount, Ldiff);
    glUniform3fv(LspecLoc, Lcount, Lspec);
    glUniform1i(NumSegments, 60);
    glUniform1i(NumStrips,60);
    glUniform4f(LineColor, 1.0f,0.0f,0.0f,0.0f);
    glUniform1f(Para_alpha,-0.369);
    glUniform1f(Para_Rmin,0.80);
    glUniform1f(Para_s,1.1);
    glUniform1f(Para_eX,0.026);
    glUniform1f(Para_eY,0.020);
    glUniform1f(Para_Rply,0.038);
    glUniform1f(Para_aply,0.453);
    glUniform1i(Para_m,75);
    glUniform1f(Para_Rholoop,22.17);
    glUniform1f(Para_Rhohair,33.77);
    glUniform2f(Para_loopmax,0.024,0.005);
    glUniform2f(Para_hairmin,0.020,0.005);
    glUniform2f(Para_Rhairspan,0.016,0.009);
    glUniform2f(Para_zhairspan,-0.003,0.057);
    glUniform2f(Para_thairspan, 0.377, 0.326);
    glUniform2f(Para_Ri, 0.001, 0.564);
    glUniform1f(Para_ti, 0.002);
    
    // 図形を描画する
    material.select(0, 0);
    shape->draw();

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}
