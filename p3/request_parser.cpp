
#include "request_parser.hpp"
#include "request.hpp"

namespace http {
namespace server {

request_parser::request_parser()
  : state_(m_begin)
{
}


request_parser::result_type request_parser::consume(request& req, char input)
{
  switch (state_)
  {
  case m_begin:
    {
      state_ = m;
      req.method.push_back(input);
      return indeterminate;
    }
  case m:
    if (input == ' ')
    {
      state_ = uri;
      return indeterminate;
    }
    else
    {
      req.method.push_back(input);
      return indeterminate;
    }
  case uri:
    if (input == ' ')
    {
      state_ = h;
      return indeterminate;
    }
    else
    {
      req.uri.push_back(input);
      return indeterminate;
    }
  case h:
    {
      state_ = t_1;
      return indeterminate;
    }
  case t_1:
    {
      state_ = t_2;
      return indeterminate;
    }
  case t_2:
    {
      state_ = p;
      return indeterminate;
    }
  case p:
    {
      state_ = slash;
      return indeterminate;
    }
  case slash:
    {
      req.http_version_major = 0;
      req.http_version_minor = 0;
      state_ = major_start;
      return indeterminate;
    }
  case major_start:
    {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      state_ = major;
      return indeterminate;
    }
  case major:
    if (input == '.')
    {
      state_ = minor_start;
      return indeterminate;
    }
    else 
    {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      return indeterminate;
    }
  case minor_start:
    {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      state_ = minor;
      return indeterminate;
    }
  case minor:
    if (input == '\r')
    {
      state_ = n_1;
      return indeterminate;
    }
    else 
    {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      return indeterminate;
    }
  case n_1:
    {
      state_ = h_start;
      return indeterminate;
    }

  case h_start:
    if (input == '\r')
    {
      state_ = n_3;
      return indeterminate;
    }
    else 
    {
      state_ = header;
      return indeterminate;
    }
  case header:
    if (input == '\r')
    {
      state_ = n_2;
      return indeterminate;
    }
    else 
    {
      return indeterminate;
    }
    
   case n_2:
    {
      state_ = h_start;
      return indeterminate;
    }
   
  case n_3:
    return good ;
  }
}



} 
} 