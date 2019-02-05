#include "common.h"
#include "arguments.h"
#include "argon2/argon2.h"
#include "argon2/base64.h"

struct argon2_validation_request {
    string base;
    string salt;
    string type;
    string hash;
    bool orphan;
    rk_sema semaphore;
};

queue<argon2_validation_request*> requests;
int requests_count = 0;
time_t timestamp = 0;
mutex requests_lock;

void queue_request(argon2_validation_request *request) {
    requests_lock.lock();
    requests.push(request);
    requests_count++;
    requests_lock.unlock();
}

argon2_validation_request *get_next_request() {
    argon2_validation_request *request = NULL;
    requests_lock.lock();
    if(!requests.empty()) {
        request = requests.front();
        requests.pop();
    }
    requests_lock.unlock();
    return request;
}

vector<string> split_argon2(const string &arg) {
    string::size_type pos, lastPos = 0, length = arg.length();
    vector<string> tokens;

    while(lastPos < length + 1)
    {
        pos = arg.find_first_of("$", lastPos);
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

argon2_validation_request *create_request(string argon, string base) {
    if(argon.empty() || base.empty())
        return NULL;

    vector<string> split_argon = split_argon2(argon);
    if(split_argon.size() != 5)
        return NULL;

    string type = split_argon[2];
    string salt = split_argon[3];

    if(type != "m=524288,t=1,p=1" && type != "m=16384,t=4,p=4")
        return NULL;

    if(salt.empty())
        return NULL;

    argon2_validation_request *request = new argon2_validation_request();
    request->type = type;
    request->salt = base64::decode(salt.c_str());
    request->base = base;
    request->orphan = false;
    rk_sema_init(&(request->semaphore), 0);

    return request;
}

void fix_query(string &query) { // replace space with +, because microhttpd seems to interpret + character as space, even if encoded
    for(int i=0;i<query.length();i++) {
        if(query[i] == ' ')
            query[i] = '+';
    }
}

const char error[] = "INVALID";
const char success[] = "VALID";

bool running = false;

static void hasher(void *memory) {
    argon2 hash_factory(memory);

    while(running) {
        argon2_validation_request *request = get_next_request();

        if(request == NULL) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        argon2profile *profile;
        if(request->type == "m=524288,t=1,p=1")
            profile = &argon2profile_1_1_524288;
        else
            profile = &argon2profile_4_4_16384;

        request->hash = hash_factory.generate_hash(*profile, request->base, request->salt);

        if(request->orphan) {
            delete request;
        }
        else {
            rk_sema_post(&(request->semaphore));
        }
    }

    free(memory);
}

static int
process_request (void *cls,
                struct MHD_Connection *connection,
                const char *url,
                const char *method,
                const char *version,
                const char *upload_data,
                size_t *upload_data_size,
                void **ptr)
{
    arguments *args = (arguments*)cls;

    string argonHash = "";
    string baseHash = "";
    struct MHD_Response *response = NULL;
    int ret;

    if (strcmp (method, "GET") != 0 || strcmp(url, "/validate") != 0)
        return MHD_NO;

    if (*ptr == NULL)
    {
        *ptr = (void*)1;
        return MHD_YES;
    }

    *ptr = NULL;

    argonHash = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "argon");
    baseHash = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "base");

    fix_query(argonHash);
    fix_query(baseHash);

    argon2_validation_request *request = create_request(argonHash, baseHash);

    bool processed = false;

    if(request == NULL)
    {
        return MHD_NO;
    }
    else
    {
        queue_request(request);
        processed = rk_sema_wait(&(request->semaphore), 5);

        if(processed && request->hash == argonHash) {
            if(args->is_verbose()) LOG("VALID: argon2 - " + argonHash + " base: " + request->base);
            response = MHD_create_response_from_buffer(strlen(success), (char *) success, MHD_RESPMEM_MUST_COPY);
        }
        else {
            if(args->is_verbose()) LOG("INVALID: argon2 - " + argonHash + " base: " + request->base);
            response = MHD_create_response_from_buffer(strlen(error), (char *) error, MHD_RESPMEM_MUST_COPY);
        }
    }

    if(processed) {
        rk_sema_destroy(&(request->semaphore));
        delete request;
    }
    else {
        request->orphan = true;
    }

    if (response == NULL)
    {
        return MHD_NO;
    }

    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
}

void shutdown(int s){
    running = false;
}

int
main (int argc, char *argv[])
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = shutdown;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    struct MHD_Daemon *d;
    timestamp = time(NULL);

    arguments args(argc, argv);

    if(args.is_help()) {
        cout << args.get_help() << endl;
        return 0;
    }

    string args_err;
    if(!args.valid(args_err)) {
        cout << args_err << endl;
        cout << "Type ArgonValidationService --help for usage information." << endl;
        return 0;
    }

    int cores = thread::hardware_concurrency() * args.intensity() / 100;
    if(cores == 0)
        cores = 1;

    LOG("Starting " + to_string(cores) + " argon2 processors on port " + to_string(args.port()) + ".");

    vector<thread*> runners;

    running = true;

    for(int i=0;i<cores;i++) {
        void *memory = malloc(argon2profile_1_1_524288.memsize);
        runners.push_back(new thread(hasher, memory));
    }

    LOG("Waiting for clients ...");

    d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD,
                          args.port(),
                          NULL, NULL, &process_request, &args, MHD_OPTION_END);
    if (d == NULL)
        return 1;

    while(running) {
        if((time(NULL) - timestamp) >= 60) {
            int tmp_reqs_count = 0;
            requests_lock.lock();
            tmp_reqs_count = requests_count;
            requests_count = 0;
            requests_lock.unlock();
            timestamp = time(NULL);
            if(args.is_verbose()) {
                LOG(to_string(tmp_reqs_count) + " validation calls per minute.");
            }
        }
        sleep(1);
    };

    for(vector<thread*>::iterator it = runners.begin();it != runners.end();++it) {
        (*it)->join();
        delete *it;
    }

    MHD_stop_daemon (d);
    return 0;
}