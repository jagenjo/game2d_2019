#ifndef _TEXTPARSER_INC
#define _TEXTPARSER_INC

/*
A classic, used everywhere to read formatted text files without using Lex
and Yacc. Written in C around 1995
Re-vamped a million times, and included in several programs
Converted to C++ in 1.7.98 to use it in the Rayman project.
*/

class TextParser
{
	char *data;
	unsigned int sl;
	unsigned int size;
public:
	unsigned int lines;

  TextParser();
  TextParser(const char*);
  ~TextParser();
  
  bool create(const char *);
  char* getword( bool to_uppercase = false );
  char* getcommaword();
  int getint();
  double getfloat();
  
  int countword(char *);
  int countwordfromhere(char *);
  int countchar(char);
  void reset();
  void destroy();
  void goback();
  void nextline();
  bool seek(const char *);
  int eof();
};

#endif









