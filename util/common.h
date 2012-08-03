#ifndef HANDLERSOCKET_COMMON_H
#define HANDLERSOCKET_COMMON_H

#define HS_PROTOCOL_OPEN        "P"
#define HS_PROTOCOL_AUTH        "A"
#define HS_PROTOCOL_INSERT      "+"
#define HS_PROTOCOL_FILTER      "F"
#define HS_PROTOCOL_WHILE       "W"
#define HS_PROTOCOL_IN          "@"

#define HS_FIND_EQUAL           "="
#define HS_FIND_LESS            "<"
#define HS_FIND_LESS_EQUAL      "<="
#define HS_FIND_GREATER         ">"
#define HS_FIND_GREATER_EQUAL   ">="

#define HS_MODIFY_UPDATE        "U"
#define HS_MODIFY_INCREMENT     "+"
#define HS_MODIFY_DECREMENT     "-"
#define HS_MODIFY_REMOVE        "D"
#define HS_MODIFY_GET_UPDATE    "U?"
#define HS_MODIFY_GET_INCREMENT "+?"
#define HS_MODIFY_GET_DECREMENT "-?"
#define HS_MODIFY_GET_REMOVE    "D?"

#define HS_CODE_NULL            0x00
#define HS_CODE_DELIMITER       0x09
#define HS_CODE_EOL             0x0a
#define HS_CODE_ESCAPE          0x10
#define HS_CODE_ESCAPE_PREFIX   0x01
#define HS_CODE_ESCAPE_ADD      0x40

#endif /* HANDLERSOCKET_COMMON_H */
