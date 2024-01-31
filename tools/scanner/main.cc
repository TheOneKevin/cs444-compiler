#include <stdio.h>

#include "lexer.h"
#include "parser.h"

extern std::string joos1w_parser_resolve_token (int yysymbol);

int main(void) {
    while (1) {
        fflush(stdin);
        char expr[256];

        if (!fgets(expr, sizeof(expr), stdin)) {
            continue;
        }

        YY_BUFFER_STATE state;

        if (!(state = yy_scan_bytes(expr, strcspn(expr, "\n")))) {
            continue;
        }

        while(int ret = yylex()) {
            printf("%d ", ret);
            printf("%s\n", joos1w_parser_resolve_token(ret).c_str());
        }

        yy_delete_buffer(state);
    }

    return 0;
}
