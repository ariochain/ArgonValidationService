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

Call with argon (share from miner) and base (calculated password based on current block) parameters using GET:

```sh
http://server:port/validate?argon=<argon>&base=<base>
```

Response: plain text VALID or INVALID.

Please be sure the requests is correct (meaning you use /validate, use proper params names and send properly formatted argon2 hashes for validation), otherwise it will reset connection. This is by design to avoid as much as possible an external attack.
