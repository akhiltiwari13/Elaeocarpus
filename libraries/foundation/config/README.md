# config

> This is an header only (INTERFACE) library that is intended to standardize config parsing across the sub-projects of this trading platform SDK.
> This is an interface only lib and the clients need to implement there own config structures and parsing logic. The api calls are standardized using (NVI/template method pattern). It is expected that the components in the critical/HOT path would use a pre-compiled configuration format (using capn' proto posibbly) which should by directly mapped with the client process' memory. For non-critical components, we can use [yaml-cpp](https://github.com/jbeder/yaml-cpp?tab=readme-ov-file) or [toml++](https://github.com/marzer/tomlplusplus) for the same.

