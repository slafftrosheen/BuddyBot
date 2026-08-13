# Halo Embodiment Gateway

MCP (Model Context Protocol) is an optional integration mechanism on the Halo Cognitive Runtime side, but it is NOT the architectural boundary.

```text
    Halo Cognitive Runtime
        |
        | RobotIntent / embodiment protocol
        v
    Halo BuddyBot Runtime
```

## MCP as an Integration Mechanism

MCP may remain a possible transport/integration mechanism on the Halo side. For example, the Halo Cognitive Runtime could use an MCP client to interact with external LLMs or agents.

```text
    LLM / Agent / MCP / Voice / App
              |
              v
        Halo Cognitive Runtime
              |
          RobotIntent
              |
              v
        Halo BuddyBot
```

## Security and Safety

- BuddyBot SafetySupervisor is authoritative.
- Halo cannot issue raw PWM.
- Halo cannot issue raw servo pulses.
- Halo cannot bypass IntentResolver.
- Halo cannot bypass ControlRouter.
- Halo cannot bypass SafetySupervisor.
- Loss of Halo connectivity must not disable local safety.
- Keep long-term memory, user preferences, and any cloud connection on the Halo Cognitive Runtime.
