#designing a trading system.

- using mkdocs to better design notes.

    - a smart order router (smorouter): this is a state machine (a finite state m/c)
    - a dumb order router (duorouter): this is a stripped down version of a smoroute with limited states & it just passes the order through to the exchange adapter.

## common features:
- config, admin commands, logging , Alerting (for prod support)
- Proc status mgmt? 
- Threading model & memory management
- Cleint/exchange session management
- Message format
-
- Exchange execution report enrichment
- Throttling of outbound messages  -> to be controlled via a throttler.
- Guard rails -> 
- Resiliency -> state persistance & recovery
- Analytics ? (needs elaboration)


## exclusive features of a smorouter
- concept of parent child order
- sophisticated FSM 
- symbol data
- market data
- limits checks
- allocation model & delta engine
- slice operations & pocket quantity
- order resurrection on trade corrects & bust handling
