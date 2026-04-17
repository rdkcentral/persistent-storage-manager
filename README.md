# CcspPsm

CcspPsm component serves as the Storage Manager in the RDK-B middleware stack. It provides centralized persistent configuration storage and retrieval services for all RDK-B components, ensuring data persistence across device reboots and factory resets. The component acts as a configuration repository that maintains system settings, user preferences, and runtime parameters in a hierarchical namespace structure. CcspPsm offers persistent storage APIs that abstract underlying storage mechanisms from other middleware components. It provides configuration validation, backup/restore capabilities, and transactional operations to ensure data integrity. The component supports both XML-based configuration files and runtime parameter management through well-defined interfaces.

At the module level, CcspPsm provides parameter get/set operations, configuration file loading/parsing, system registry management, and HAL integration for platform-specific storage requirements. It integrates with RBus messaging systems and other IPC methods to serve configuration requests from other RDK-B components and external management systems.

```mermaid
graph LR

    subgraph "External Systems"
        RemoteMgmt["Remote Management"]
        LocalUI["Local Web UI"]
    end

    subgraph "RDK-B Platform"
        subgraph "Remote Management Agents"
            ProtocolAgents["Protocol Agents<br>( TR-069, WebPA, USP etc.)"]
        end

        rdkbComponent["Other RDK-B Components<br>(PNM,WAN Manager etc.)"]
        CcspPsm["CCSP PSM"]

        SyscfgDB[(Syscfg DB)]
    end

    %% External connections
    RemoteMgmt -->|TR-069/WebPA/TR-369| ProtocolAgents
    LocalUI -->|HTTP/HTTPS| ProtocolAgents
    
    ProtocolAgents -->|IPC| rdkbComponent
    
    rdkbComponent -->|APIs| CcspPsm
    CcspPsm -->|APIs| rdkbComponent
    CcspPsm -->|APIs| SyscfgDB

    classDef external fill:#fff3e0,stroke:#ef6c00,stroke-width:2px;
    classDef CcspPsm fill:#e3f2fd,stroke:#1976d2,stroke-width:3px;
    classDef rdkbComponent fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px;
    classDef system fill:#fce4ec,stroke:#c2185b,stroke-width:2px;

    class RemoteMgmt,LocalUI external;
    class CcspPsm CcspPsm;
    class ProtocolAgents,rdkbComponent rdkbComponent;
    class SyscfgDB system;
```

**Key Features & Responsibilities**: 

- **Persistent Configuration Storage**: Manages hierarchical parameter storage with support for different data types (string, integer, boolean, datetime, binary) and ensures data persistence across reboots and power cycles
- **Configuration File Management**: Handles XML-based configuration file parsing, loading, validation, and storage with support for default configurations, current settings, and backup files
- **Transaction Support**: Provides atomic operations for configuration updates with rollback capabilities to ensure data consistency during complex configuration changes
- **Factory Reset Support**: Implements factory reset functionality with the ability to restore default configurations while preserving critical system parameters
- **IPC Support**: Offers RBus interfaces for parameter access, enabling integration with CCSP components and modern RBus-based services
- **HAL Integration**: Provides platform-specific storage abstraction through HAL APIs, allowing adaptation to different hardware platforms and storage mechanisms
- **Configuration Validation**: Validates parameter values, types, and constraints before persistence to ensure system stability and prevent invalid configurations
- **Backup and Restore**: Supports configuration backup creation and restoration capabilities for system recovery and migration scenarios

## Design

The CcspPsm component follows a layered architecture design that separates concerns between configuration management, storage abstraction, and inter-process communication. The design centers around three core modules: PsmSysRegistry for runtime parameter management, PsmFileLoader for configuration file handling, and the SSP (Sub-System Process) layer for IPC integration. This modular approach enables independent evolution of storage mechanisms, communication protocols, and configuration formats while maintaining stable interfaces.

The architecture implements a dual-storage strategy where runtime parameters are managed in-memory through the SysRegistry module with periodic flushes to persistent storage, while configuration files are handled through the FileLoader module that parses, validates, and manages XML-based configuration data. The design incorporates robust error handling, transaction support, and rollback mechanisms to ensure data integrity during configuration updates. The component employs both synchronous and asynchronous operations, with critical configuration changes handled synchronously to prevent data loss.

North-bound interactions with other RDK-B components are managed through standardized RBus interfaces and other IPC methods that provide parameter get/set operations, event notifications, and bulk configuration operations. The design abstracts the underlying storage complexity from clients, presenting a unified hierarchical namespace for all configuration parameters. South-bound interactions with the HAL layer are handled through well-defined APIs that abstract platform-specific storage mechanisms, enabling portability across different hardware platforms while maintaining consistent behavior.

The IPC mechanisms are integrated with RBus providing modern, high-performance messaging for components. The design includes message queuing, event notification, and subscription mechanisms to support real-time configuration updates and system monitoring. Data persistence is achieved through a combination of immediate writes for critical parameters and batched writes for performance optimization, with configurable flush intervals and emergency write triggers.

```mermaid
flowchart TD
    subgraph CcspPsm
        direction LR
        subgraph "SSP Layer"
            MainController[Main Controller]
            CFMInterface[CFM Interface]
        end
        
        subgraph "PsmSysRegistry"
            RegInterface[Registry Interface]
            RegControl[Registry Control]
            RegStorage[Registry Storage]
            RegBase[Registry Base]
            RegStates[Registry States]
        end
        
        subgraph "PsmFileLoader"
            FileInterface[File Loader Interface]
            FileControl[File Loader Control]
            XMLParser[XML Parser]
            FileOperations[File Operations]
            LoaderStates[Loader States]
        end
    end

    MainController --> RegInterface
    MainController --> FileInterface
    
    CFMInterface --> MainController
    
    RegInterface --> RegControl
    RegControl --> RegStorage
    RegControl --> RegStates
    RegStorage --> RegBase
    
    FileInterface --> FileControl
    FileControl --> XMLParser
    FileControl --> FileOperations
    FileControl --> LoaderStates
    
    RegStorage --> FileInterface
    FileOperations --> RegStorage
    RegStates --> FileOperations
```


### Prerequisites and Dependencies

**RDK-B Platform and Integration Requirements:** 

- **RDK-B Components**: CcspCommonLibrary, RBus framework, systemd for service management, syscfg utility for platform configuration
- **HAL Dependencies**: Platform Storage HAL, Syscfg HAL for platform-specific configuration access
- **Systemd Services**: filesystem mount points for the PSM configuration path and /tmp must be available
- **Message Bus**: RBus integration uses the PSM RBus methods registered by the component (`SetPSMRecordValue()`, `GetPSMRecordValue()`, `DeletePSMRecord()`, and `GetPSMRecordName()`). No fixed `Device.X_CISCO_COM_PSM.` registration prefix is used.
- **Configuration Files**: Default component paths are rooted at `/psm/config/`, with configuration files such as `psm_def_cfg.xml.gz` (default/base configuration) and `psm_cur_cfg.xml.gz` (current persistent configuration)
- **Startup Order**: Must initialize after filesystem services that make `/psm/config/` available, before other CCSP components that depend on configuration data

<br>

**Threading Model:** 

CcspPsm implements a hybrid threading model combining single-threaded main processing with worker threads for I/O operations and periodic maintenance tasks. The main thread handles all IPC message processing, parameter validation, and configuration updates to ensure thread safety and avoid race conditions in critical configuration operations.

- **Threading Architecture**: Single-threaded main event loop with dedicated worker threads for file I/O and backup operations
- **Main Thread**: Handles RBus message processing, parameter validation, configuration updates, and maintains the in-memory parameter cache
- **Main worker Threads**: 
  - **File I/O Thread**: Manages XML configuration file reading, writing, and compression operations
  - **Backup Thread**: Handles periodic configuration backups and cleanup of temporary files
  - **Flush Thread**: Performs periodic cache flushes to persistent storage at configurable intervals
- **Synchronization**: Uses mutex locks for cache access, semaphores for worker thread coordination, and atomic operations for critical flags

### Component State Flow

**Initialization to Active State**

The CcspPsm component follows a structured initialization sequence that establishes all necessary subsystems before entering the active operational state. The process begins with basic system setup, progresses through configuration loading and validation, establishes IPC connections, and finally registers with the message bus systems. Each initialization phase includes error handling and rollback mechanisms to ensure system stability.

```mermaid
sequenceDiagram
    autonumber
    participant System
    participant PSM
    participant Config
    participant Validator
    participant RBus
    participant HAL
    participant Clients

    System->>System: Start [*] → Initializing
    Note right of System: Initialize logging subsystem<br>Allocate memory pools<br>Setup signal handlers<br>Create worker threads

    System->>PSM: Service Start →<br>LoadingDefaults
    Note right of PSM: Load default XML config<br>Read configuration files<br>Initialize parameter cache

    PSM->>Config: Initialize Logging & Memory →<br>ParseConfiguration
    Config->>Validator: Load Default XML Config →<br>ValidatingConfig
    Note right of Validator: Parse & validate parameters<br>Check parameter constraints<br>Build parameter registry

    Validator->>RBus: Configuration Validated →<br>RegisteringRBus
    RBus->>HAL: RBus Registration Complete →<br>ConnectingHAL
    HAL->>System: HAL Connections Established →<br>Active

    Note right of System: Process parameter requests<br>Handle configuration updates<br>Manage cache consistency<br>Monitor system health

    System->>System: Parameter Update Request →<br>RuntimeConfigChange
    System->>System: Configuration Updated →<br>Active
    System->>System: Periodic Backup Trigger →<br>BackupOperation
    System->>System: Backup Complete →<br>Active

    System->>System: Stop Signal Received →<br>Shutdown → [*]
```

**Runtime State Changes and Context Switching**

During normal operation, CcspPsm handles various runtime state changes triggered by external events, configuration updates, and maintenance operations. The component maintains internal state consistency while supporting concurrent operations from multiple clients.

**State Change Triggers:**

- Parameter update requests from RBus clients trigger cache updates and persistence operations
- Factory reset commands transition the system to configuration restoration state with default value loading
- Backup timer expiration triggers background backup operations without affecting normal parameter access
- Storage errors initiate error recovery procedures with automatic retry and fallback mechanisms

**Context Switching Scenarios:**

- Configuration file updates require temporary service suspension during atomic file replacement operations
- Factory reset operations switch to restricted mode where only essential parameters remain accessible
- Storage fault detection triggers failover to backup storage locations with automatic recovery attempts

### Call Flow

**Initialization Call Flow:**

```mermaid
sequenceDiagram
    participant Systemd as Systemd Service Manager
    participant PSM as CcspPsm Process
    participant RBus as RBus Framework
    participant HAL as Platform HAL

    Systemd->>PSM: Start PsmSsp Process
    PSM->>PSM: Load Default Configuration
    PSM->>PSM: Initialize Parameter Cache
    PSM->>RBus: Register RBus Interface
    RBus-->>PSM: Registration Confirmed
    PSM->>HAL: Connect Platform APIs
    HAL-->>PSM: HAL Connection Established
    PSM->>Systemd: Notify Service Ready
```

**Request Processing Call Flow:**

```mermaid
sequenceDiagram
    participant Client as RDK-B Component
    participant PSM as CcspPsm Main Thread
    participant Cache as Parameter Cache
    participant Storage as Persistent Storage
    participant HAL as Platform HAL

    Client->>PSM: Parameter Get Request (RBus)
    PSM->>Cache: Check Parameter Cache
    Cache-->>PSM: Cache Hit/Miss Result
    alt Cache Miss
        PSM->>Storage: Load from Persistent Storage
        Storage-->>PSM: Parameter Value Retrieved
        PSM->>Cache: Update Cache Entry
    end
    PSM-->>Client: Parameter Value Response
    
    Client->>PSM: Parameter Set Request (RBus)
    PSM->>PSM: Validate Parameter Value
    PSM->>Cache: Update Cache Entry
    PSM->>Storage: Schedule Persistent Write
    PSM->>HAL: Platform-Specific Update (if needed)
    HAL-->>PSM: Update Confirmation
    PSM-->>Client: Set Operation Response
```

## Internal Modules

The CcspPsm component is structured into three primary modules that handle different aspects of persistent storage management. The PsmSysRegistry module manages the runtime parameter registry and in-memory caching, providing high-performance parameter access and validation. The PsmFileLoader module handles XML configuration file operations including parsing, validation, and file management for default, current, and backup configurations. The SSP (Sub-System Process) module provides the interface layer for RBus communications, HAL integration, and system service management.

| Module/Class | Description | Key Files |
|-------------|------------|-----------|
| PsmSysRegistry | Manages runtime parameter registry, in-memory cache operations, parameter validation, and provides atomic transaction support for configuration updates | `psm_sysro_interface.c`, `psm_sysro_control.c`, `psm_sysro_storage.c`, `psm_sysro_base.c` |
| PsmFileLoader | Handles XML configuration file parsing, validation, loading of default configurations, and manages backup/restore operations with compression support | `psm_flo_interface.c`, `psm_flo_parse.c`, `psm_flo_operation.c`, `psm_flo_control.c` |
| SSP Layer | Provides RBus interface implementations, HAL integration layer, main process control, and system service management including signal handling | `ssp_main.c`, `ssp_rbus.c`, `ssp_cfmif.c`, `psm_hal_apis.c` |


## Component Interactions

CcspPsm serves as the central configuration repository for the RDK-B middleware ecosystem, interfacing with numerous components through standardized IPC mechanisms. The component provides RBus interfaces to accommodate modern RBus-based services and CCSP components. It integrates with platform HAL layers for hardware-specific storage operations and communicates with external management systems through TR-069 and web interfaces.

### Interaction Matrix

| Target Component/Layer | Interaction Purpose | Key APIs/Endpoints |
|------------------------|--------------------|--------------------|
| **RDK-B Middleware Components** | | |
| CcspPandM | Configuration parameter management, system status reporting, factory reset coordination | `GetPSMRecordValue()`, `SetPSMRecordValue()` |
| CcspTr069Pa | TR-069 parameter persistence, ACS configuration storage, device provisioning data | `getParameterValues()`, `setParameterValues()`, `addObject()` |
| OneWifi | WiFi configuration persistence, radio settings, access point parameters | `Device.WiFi.` namespace parameters, WiFi credential storage |
| CcspDmCli | Command-line data model access, diagnostic parameter retrieval, testing interface | RBus method calls for parameter tree navigation |
| **System & HAL Layers** | | |
| Syscfg HAL | Platform-specific configuration parameter access, hardware-dependent settings | `syscfg_get()`, `syscfg_set()`, `syscfg_commit()` |
| File System | Configuration file storage, backup operations, temporary file management | `/nvram/psm_cfg.xml`, `/tmp/psm_backup.xml` |
| Platform Services | System integration, service lifecycle management, resource monitoring | Systemd unit file control, process monitoring |

### IPC Flow Patterns

**Primary IPC Flow - Parameter Get/Set Operations:**

```mermaid
sequenceDiagram
    participant Client as RDK-B Component
    participant RBus as RBus Framework
    participant PSM as CcspPsm Main Process
    participant Cache as Parameter Cache
    participant Storage as Persistent Storage

    Client->>RBus: Parameter Get Request
    RBus->>PSM: Route Get Method Call
    PSM->>Cache: Check In-Memory Cache
    alt Cache Hit
        Cache-->>PSM: Return Cached Value
    else Cache Miss
        PSM->>Storage: Load from Persistent Store
        Storage-->>PSM: Parameter Value
        PSM->>Cache: Update Cache Entry
    end
    PSM-->>RBus: Parameter Value Response
    RBus-->>Client: Return Parameter Value
    
    Client->>RBus: Parameter Set Request
    RBus->>PSM: Route Set Method Call
    PSM->>PSM: Validate Parameter Value & Type
    PSM->>Cache: Update In-Memory Cache
    PSM->>Storage: Schedule Persistent Write
    PSM-->>RBus: Set Operation Success
    RBus-->>Client: Acknowledge Set Operation
```

**Event Notification Flow:**

```mermaid
sequenceDiagram
    participant Client as Configuration Client
    participant PSM as CcspPsm Process
    participant RBus as RBus Event System
    participant Sub1 as CcspPandM
    participant Sub2 as CcspTr069Pa

    Client->>PSM: Set Parameter Request
    PSM->>PSM: Update Parameter Value
    PSM->>PSM: Check for Value Change
    alt Value Changed
        PSM->>RBus: Publish ParameterValueChanged Event
        RBus->>Sub1: Event Notification (RBus)
        RBus->>Sub2: Event Notification (IPC)
        Sub1-->>PSM: Event Acknowledgment (if required)
        Sub2-->>PSM: Event Acknowledgment (if required)
    end
    PSM-->>Client: Set Operation Complete
```

## Implementation Details

### Major HAL APIs Integration

CcspPsm integrates with platform-provided Syscfg APIs for persistent storage and configuration access. These APIs are consumed by CcspPsm code in `ssp_cfmif.c`, but their implementation and ownership are external to this repository.

**Core HAL APIs:**

| HAL API | Purpose | Parameters | Return Values | Implementation / Ownership |
|---------|---------|------------|---------------|---------------------------|
| `syscfg_get()` | Retrieve a platform-specific configuration parameter from persistent storage | 4-argument form: `syscfg_get(NULL, key, buf, size)` | `0` on success, non-zero on error | External Syscfg API consumed in `ssp_cfmif.c`; not implemented in this repository |
| `syscfg_unset()` | Remove a platform-specific configuration parameter from persistent storage | 2-argument form: `syscfg_unset(NULL, key)` | `0` on success, non-zero on error | External Syscfg API consumed in `ssp_cfmif.c`; not implemented in this repository |
| `PsmHal_GetCustomParams()` | Retrieve platform-specific default parameters for initial configuration | `PsmHalParam_t** params, int* cnt` | `0` on success, `-1` on failure or no custom parameters | `psm_hal_apis.c` |

### Key Implementation Logic

- **Parameter Registry Engine**: Core parameter management logic implemented in `psm_sysro_interface.c` with hash-based parameter lookup, type validation, and atomic update operations. Main registry implementation in `psm_sysro_base.c` with parameter tree management and namespace handling. State transition handlers in `psm_sysro_states.c` for parameter lifecycle management and event processing.
  
- **Configuration File Processing**: XML configuration file parsing and validation handled through libxml2 integration with custom schema validation. Configuration parsing engine in `psm_flo_parse.c` with XML schema validation and parameter extraction. File operation management in `psm_flo_operation.c` with atomic file updates and backup creation. Configuration loading state machine in `psm_flo_states.c` for startup and runtime configuration management.
  
- **Error Handling Strategy**: Comprehensive error detection and recovery mechanisms for storage failures, parsing errors, and IPC communication issues. HAL error code mapping with automatic retry mechanisms for transient failures. Configuration validation with rollback support for invalid parameter updates. Graceful degradation with emergency read-only mode when persistent storage fails.
  
- **Logging & Debugging**: Multi-level logging system with component-specific categories and runtime verbosity control. Parameter operation logging with value change tracking and audit trail. File operation tracing with performance metrics and error diagnostics. RBus message tracing for IPC debugging and performance analysis.

### Key Configuration Files

| Configuration File | Purpose | Key Parameters | Default Values | Override Mechanisms |
|--------------------|---------|---------------|----------------|--------------------|
| `/psm/config/psm_def_cfg.xml.gz` | Default/base configuration (maps to `PSM_DEF_DEF_FILE_NAME`) | All default parameter values | Factory defaults | Replacement via update procedure |
| `/psm/config/psm_cur_cfg.xml.gz` | Current persistent configuration (maps to `PSM_DEF_CUR_FILE_NAME`) | All runtime parameters | Populated from default config | Parameter Set operations, configuration import |
| `/psm/config/psm_bak_cfg.xml.gz` | Backup persisted configuration archive (maps to `PSM_DEF_BAK_FILE_NAME`) | Previous stable configuration set | Previous persisted current configuration | Automatic backup/restore workflow |
| `/tmp/bbhm_cur_cfg.xml` | Runtime working copy of current active configuration (`PSM_CUR_CONFIG_FILE_NAME` in `ssp_cfmif.c`) | All runtime parameters | Populated from persistent config | Parameter Set operations |
| `/nvram/bbhm_bak_cfg.xml` | Runtime backup configuration (`PSM_BAK_CONFIG_FILE_NAME` in `ssp_cfmif.c`) | Previous stable configuration | Previous current config | Automatic backup on major changes |