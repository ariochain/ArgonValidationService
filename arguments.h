//
// Created by Haifa Bogdan Adnan on 05/02/2019.
//

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

class arguments {
public:
    arguments(int argc, char *argv[]);

    bool valid(string &error);

    bool is_help();
    bool is_verbose();

    double intensity();
    int port();

    string get_help();

private:
    void __init();
    vector<string> __parse_multiarg(const string &arg);

    string __error_message;
    bool __error_flag;

    int __help_flag;
    int __verbose_flag;

    double __intensity;
    int __port;

    static string __argv_0;
};

#endif
