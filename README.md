# ArgonValidationService
Service for validating argon2 hashes generated while mining arionum coins.

## Usage
```sh

ArgonValidationService --port <port> --intensity <intensity> --verbose

Parameters:
   --help: show this help text
   --verbose: print more informative text during run
   --port <port>: port on which to listen for clients, defaults to 2000
   --intensity: percentage of available CPU cores to use
                    value from 0 (disabled) to 100 (full load)
                    this is optional, defaults to 100
                    
```

## API

Call with argon and base parameters using GET:

```sh
http://server:port/validate?argon=<argon>&base=<base>
```

Response: plain text VALID or INVALID.