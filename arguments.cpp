//
// Created by Haifa Bogdan Adnan on 05/02/2019.
//

#include "common.h"
#include "arguments.h"

arguments::arguments(int argc, char **argv) {
    __argv_0 = argv[0];

    __init();

    int c = 0;
    char buff[50];

    while (c != -1)
    {
        static struct option options[] =
        {
            {"help", no_argument,  NULL, 'h'},
            {"verbose", no_argument, NULL, 'v'},
            {"port", required_argument, NULL, 'p'},
            {"intensity", required_argument, NULL, 'i'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hvp:i:",
                         options, &option_index);

        switch (c)
        {
            case -1:
            case 0:
                break;
            case 1:
                sprintf(buff, "%s: invalid arguments",
                                  argv[0]);
                __error_message = buff;
                __error_flag = true;
                c = -1;
                break;
            case 'h':
                __help_flag = 1;
                break;
            case 'v':
                __verbose_flag = 1;
                break;
            case 'p':
                if(strcmp(optarg, "-h") == 0 || strcmp(optarg, "--help") == 0) {
                    __help_flag = 1;
                }
                else {
                    __port = atoi(optarg);
                }
                break;
            case 'i':
                if(strcmp(optarg, "-h") == 0 || strcmp(optarg, "--help") == 0) {
                    __help_flag = 1;
                }
                else {
                    __intensity = atof(optarg);
                }
                break;
            case ':':
                __error_flag = true;
                break;
            default:
                __error_flag = true;
                break;
        }
    }

	if (optind < argc)
    {
        sprintf(buff, "%s: invalid arguments",
                          argv[0]);
        __error_message = buff;
        __error_flag = true;
    }
}

bool arguments::valid(string &error) {
    error = __error_message;

    if(__error_flag)
        return false;

    if (__intensity < 0 || __intensity > 100) {
        error = "Intensity must be between 0 - disabled and 100 - full load.";
        return false;
    }

    if (__port < 10) {
        error = "Port should be a positive number bigger than 10.";
        return false;
    }

    return true;
}

bool arguments::is_help() {
    return __help_flag == 1;
}

bool arguments::is_verbose() {
    return __verbose_flag == 1;
}

int arguments::port() {
    return __port;
}

double arguments::intensity() {
    return __intensity;
}

string arguments::get_help() {
    return
            "\nArgonValidationService v." ArgonValidationService_VERSION_MAJOR "." ArgonValidationService_VERSION_MINOR "." ArgonValidationService_VERSION_REVISION "\n"
            "Copyright (C) 2019 Haifa Bogdan Adnan\n"
            "\n"
            "Usage:\n"
            "       ArgonValidationService --port <port> --intensity <intensity> --verbose\n"
            "\n"
            "Parameters:\n"
            "   --help: show this help text\n"
            "   --verbose: print more informative text during run\n"
            "   --port <port>: port on which to listen for clients, defaults to 2000\n"
            "   --intensity: percentage of available CPU cores to use\n"
            "                    value from 0 (disabled) to 100 (full load)\n"
            "                    this is optional, defaults to 100\n"
            "\n"
            ;
}

void arguments::__init() {
    __help_flag = 0;
    __verbose_flag = 0;
    __intensity = 100;
    __port = 2000;
    __error_flag = false;
}

string arguments::__argv_0 = "./";

vector<string> arguments::__parse_multiarg(const string &arg) {
    string::size_type pos, lastPos = 0, length = arg.length();
    vector<string> tokens;

    while(lastPos < length + 1)
    {
        pos = arg.find_first_of(",", lastPos);
        if(pos == std::string::npos)
        {
            pos = length;
        }

        if(pos != lastPos)
            tokens.push_back(string(arg.c_str()+lastPos,
                                        pos-lastPos ));

        lastPos = pos + 1;
    }

    return tokens;
}
