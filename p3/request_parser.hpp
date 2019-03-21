

#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include <tuple>

namespace http {
namespace server {
	
struct request;
	
class request_parser
{
public:
  request_parser();

  enum result_type { good, indeterminate };

  template <typename InputIterator>
  std::tuple<result_type, InputIterator> parse(request& req,
      InputIterator begin, InputIterator end)
  {
    while (begin != end)
    {
      result_type result = consume(req, *begin++);
      if (result == good)
        return std::make_tuple(result, begin);
    }
    return std::make_tuple(indeterminate, begin);
  }

private:

  result_type consume(request& req, char input);


  enum state
  {
    m_begin,
    m,
    uri,
    h,
    t_1,
    t_2,
    p,
    slash,
    major_start,
    major,
    minor_start,
    minor,
    n_1,
	h_start,
    header,
    n_2,
    n_3
  } state_;
};
} 
} 

#endif // HTTP_SERVER_HPP