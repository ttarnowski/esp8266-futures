#pragma once

class ErrorBase {
public:
  virtual ~ErrorBase() = 0;
  virtual operator const char *() = 0;
};

ErrorBase::~ErrorBase() {}
