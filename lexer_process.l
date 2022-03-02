
%option nounput yylineno
%option noyywrap yylineno


%%
[ \t]*			            ;
#[a-z | 0-9 | A-Z | : | . | _ | # | ( | ) | ! | / | ? | " | ' | { | } | ’]+               ;

"global"                    return 1;
"extern"                    return 2;
"section"                   return 3;
"halt"                      return 15;
"int"                       return 16;
r0|r1|r2|r3|r4|r5|r6|r7     return 17;
"pc"|"sp"                   return 44;
"psw"                       return 56;
"iret"                      return 20;
"call"                      return 21;
"ret"                       return 22;
"jmp"                       return 27;
"jeq"                       return 28;
"jne"                       return 29;
"jgt"                       return 30;
"add"                       return 31;
"sub"                       return 32;
"mul"                       return 33;
"div"                       return 34;
"cmp"                       return 35;
"not"                       return 36;
"and"                       return 37;
"or"                        return 38;
"xor"                       return 39;
"test"                      return 40;
"xchg"                      return 41;
"shl"                       return 42;
"shr"                       return 43;
"ldr"                       return 45;
"str"                       return 46;
"push"                      return 48;
"pop"                       return 49;

\n                          return 4;
","			                return 5;
"*"                         return 6;
"word"                      return 11;
"skip"                      return 12;
":"                         return 13;
"equ"                       return 14;
"%"                         return 23;
"["                         return 24;
"]"                         return 25;
"+"                         return 26;
"$"                         return 47;


0[xX][0-9a-fA-F]+                                               return 55;
[-][0-9][0-9]*				                                    return 7;
[0-9][0-9]*				                                        return 8;
[A-F|a-f]+                                                      return 52;
[a-z|A-Z]                                                       return 9;
[a-z|A-Z]*                                                      return 10;
[a-z|A-Z|_|#|(|)|!|/|?|"|{|}|'|0-9]                             return 53;
[a-z|A-Z|_|#|(|)|!|/|?|"|{|}|'|0-9]*                            return 54;


.			                return 50;

%%
