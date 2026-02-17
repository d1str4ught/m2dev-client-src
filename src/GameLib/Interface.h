#pragma once

// 2004.07.05.myevan.궁신탄영 맵에 끼이는 문제해결
class IBackground : public CSingleton<IBackground> {
public:
  IBackground() {}
  virtual ~IBackground() {}

  virtual bool IsBlock(int x, int y) = 0;
};