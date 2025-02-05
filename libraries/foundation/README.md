# Foundation Libs:

- These are the foundational libs for building this Trading Platform SDK. The libraries in this directory are intended to be used as primary building blocks for various components of this trading system.
- Most of the libraries here should ideally be a wrapper over a high-performant external library in-order to facilitate decoupling with third party libs.

## Details

### config: 

> The intent with the config library is that all the sub-modules of the platform could use this to define and parse their configurations respectively. This will primarily be a wrapper over libs like (yaml-cpp or toml-cpp or cap'n proto) to decouple the architecture from external libraries/dependencies.

>  Key Design Goals
  -  Type-Safety: Each sub-project (e.g., market data, order execution) has its own config struct.
  -  Zero Runtime Overhead: Compile-time resolution for config access in hot paths.
  -  Immutable Configs: Loaded once during initialization and never modified.
  -  No Virtual Functions: Use templates and CRTP for static polymorphism.

> Tradeoffs

  - Pros:
       1. Type-safe, zero-overhead access in hot paths.
       2. No virtual function calls.
       3. Sub-projects can evolve independently.

  - Cons:
       1. Slightly increased compile-time due to templates.
       2. std::any has minor storage overhead (acceptable if configs are loaded once).

### logger: A wrapper over spdlog
