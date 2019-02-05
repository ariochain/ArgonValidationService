//
// Created by Haifa Bogdan Adnan on 17/08/2018.
//

#ifndef BASE64_H
#define BASE64_H

class base64 {
public:
    static void encode(const char *input, int input_size, char *output);
    static string decode(const char *encoded_string);
};

#endif //BASE64_H
