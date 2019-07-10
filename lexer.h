#ifndef MAXC_LEXER_H
#define MAXC_LEXER_H

class Lexer {
  public:
    Token &run(std::string src);

  private:
    Token token;
    void save(int, int);
    location_t &get();

    location_t saved_loc;
};

#endif
