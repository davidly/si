//
// System Information
// Windows console app to show CPU, RAM, network, and disk information about the local machine.
// The 8-bit string A versions of Win32 API are used to maximize Win98 compatibility.
// Any APIs not available out of the box on Win98 are dynamically loaded and opportunistically used.
// Lots of structs and enums are duplicated below so the app builds with VS 6.0 and
// its headers from long ago. That enables the .exe to run on Windows 98 and later.
// Building with recent tools yields binaries that won't even run on XP.
//
// When building with vs6 I grabbed 2003 versions of these networking headers:
//    ipexport.h, iphlpapi.h, iprtrmib.h, iptypes.h
//
// Build on Windows 11 to target various platforms:
//
//    32-bit Windows with VS2019 / Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30140 for x86
//        (environment setup with vcvars32.bat)
//        mkdir x86
//        cl /I..\djl /nologo si.cxx /EHac /Zi /O2i /Gy /Fe: x86\si.exe /link /OPT:REF /incremental:no
//
//    64-bit Windows with VS2022 / Microsoft (R) C/C++ Optimizing Compiler Version 19.31.31104 for x64
//        (environment setup with vcvars64.bat)
//        cl /nologo si.cxx /I:.\ /EHac /Zi /O2i /Gy /D_AMD64_ /link /OPT:REF /incremental:no
//
//    32-bit Windows 98 and later with VS6 / Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 12.00.8168 for 80x86
//        (environment setup to use the old compiler/include/lib)
//        mkdir win98
//        cl /nologo si.cxx /Ox /Zi /Fewin98\si.exe /link /incremental:no
//
//    64-bit Windows 11 on Arm64 with VS2022 / Microsoft (R) C/C++ Optimizing Compiler Version 19.34.31932 for ARM64
//        mkdir arm64
//        cl /nologo si.cxx /I:.\ /DDEBUG /EHac /Zi /O2i /Gy /Fe: arm64\si.exe /link /OPT:REF /incremental:no
//

#include <stdio.h>
#include <process.h>
#include <direct.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#if _MSC_VER > 1200
#include <intrin.h>       // needed for __cpuid
#endif

#include <windows.h>
#include <winreg.h>
#include <winioctl.h>
#include <iphlpapi.h>

#include <vector>
using namespace std;

bool g_fullInformation = false;
const _int64 OneMeg = (_int64) 1024 * (_int64) 1024;

// Microsoft accidentally omitted this from headers decades ago...

typedef struct _PROCESSOR_POWER_INFORMATION_FAKE {
  ULONG Number;
  ULONG MaxMhz;
  ULONG CurrentMhz;
  ULONG MhzLimit;
  ULONG MaxIdleState;
  ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION_FAKE, *PPROCESSOR_POWER_INFORMATION_FAKE;

#ifndef PDCAP_D0_SUPPORTED

    typedef enum _POWER_INFORMATION_LEVEL {
        SystemPowerPolicyAc,
        SystemPowerPolicyDc,
        VerifySystemPolicyAc,
        VerifySystemPolicyDc,
        SystemPowerCapabilities,
        SystemBatteryState,
        SystemPowerStateHandler,
        ProcessorStateHandler,
        SystemPowerPolicyCurrent,
        AdministratorPowerPolicy,
        SystemReserveHiberFile,
        ProcessorInformation,
        SystemPowerInformation,
        ProcessorStateHandler2,
        LastWakeTime,
        LastSleepTime,
        SystemExecutionState,
        SystemPowerStateNotifyHandler,
        ProcessorPowerPolicyAc,
        ProcessorPowerPolicyDc,
        VerifyProcessorPowerPolicyAc,
        VerifyProcessorPowerPolicyDc,
        ProcessorPowerPolicyCurrent,
        SystemPowerStateLogging,
        SystemPowerLoggingEntry,
        SetPowerSettingValue,
        NotifyUserPowerSetting,
        PowerInformationLevelUnused0,
        PowerInformationLevelUnused1,
        SystemVideoState,
        TraceApplicationPowerMessage,
        TraceApplicationPowerMessageEnd,
        ProcessorPerfStates,
        ProcessorIdleStates,
        ProcessorCap,
        SystemWakeSource,
        SystemHiberFileInformation,
        TraceServicePowerMessage,
        ProcessorLoad,
        PowerShutdownNotification,
        MonitorCapabilities,
        SessionPowerInit,
        SessionDisplayState,
        PowerRequestCreate,
        PowerRequestAction,
        GetPowerRequestList,
        ProcessorInformationEx,
        NotifyUserModeLegacyPowerEvent,
        GroupPark,
        ProcessorIdleDomains,
        WakeTimerList,
        SystemHiberFileSize,
        PowerInformationLevelMaximum
    } POWER_INFORMATION_LEVEL;

#endif

// Use fake versions of these since their existence and definitions change between versions of the sdk

typedef enum _MACHINE_ATTRIBUTES_FAKE {
    UserEnabledFake = 0x00000001,
    KernelEnabledFake = 0x00000002,
    Wow64ContainerFake = 0x00000004
} MACHINE_ATTRIBUTES_FAKE;

typedef struct _PROCESS_MACHINE_INFORMATION_FAKE {
    USHORT             ProcessMachine;
    USHORT             Res0;
    MACHINE_ATTRIBUTES_FAKE MachineAttributes;
} PROCESS_MACHINE_INFORMATION_FAKE;

typedef enum _PROCESS_INFORMATION_CLASS_FAKE {
    ProcessMemoryPriorityFake,
    ProcessMemoryExhaustionInfoFake,
    ProcessAppMemoryInfoFake,
    ProcessInPrivateInfoFake,
    ProcessPowerThrottlingFake,
    ProcessReservedValue1Fake,
    ProcessTelemetryCoverageInfoFake,
    ProcessProtectionLevelInfoFake,
    ProcessLeapSecondInfoFake,
    ProcessMachineTypeInfoFake,
    ProcessInformationClassMaxFake
} PROCESS_INFORMATION_CLASS_FAKE;

// Enable building 32-bit binaries with VS6 tools so the binary can run in Win98.
// That means replicating all of these definitions.

#if _MSC_VER == 1200

    typedef HANDLE DPI_AWARENESS_CONTEXT;
    #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)

    #ifndef LTP_PC_SMT
        #define LTP_PC_SMT 0x1
    #endif

    #define _countof( x ) ( sizeof( x ) / sizeof( x[0] ) )

    typedef unsigned int ULONG_PTR;
    typedef HRESULT NTSTATUS;
    typedef LONG LSTATUS;

    #define PROCESSOR_ARCHITECTURE_AMD64 9

    #ifndef PROCESSOR_ARCHITECTURE_ARM64
        #define PROCESSOR_ARCHITECTURE_ARM64 12
    #endif

    typedef HANDLE HMONITOR;
    typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

    #define IOCTL_STORAGE_QUERY_PROPERTY                CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

    typedef struct _DEVICE_TRIM_DESCRIPTOR {
        DWORD       Version;          // keep compatible with STORAGE_DESCRIPTOR_HEADER
        DWORD       Size;             // keep compatible with STORAGE_DESCRIPTOR_HEADER
        BOOLEAN     TrimEnabled;
    } DEVICE_TRIM_DESCRIPTOR, *PDEVICE_TRIM_DESCRIPTOR;

    typedef struct _DEVICE_SEEK_PENALTY_DESCRIPTOR {
        DWORD       Version;          // keep compatible with STORAGE_DESCRIPTOR_HEADER
        DWORD       Size;             // keep compatible with STORAGE_DESCRIPTOR_HEADER
        BOOLEAN     IncursSeekPenalty;
    } DEVICE_SEEK_PENALTY_DESCRIPTOR, *PDEVICE_SEEK_PENALTY_DESCRIPTOR;

    typedef enum _STORAGE_QUERY_TYPE {
        PropertyStandardQuery = 0,          // Retrieves the descriptor
        PropertyExistsQuery,                // Used to test whether the descriptor is supported
        PropertyMaskQuery,                  // Used to retrieve a mask of writeable fields in the descriptor
        PropertyQueryMaxDefined     // use to validate the value
    } STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

    typedef enum _STORAGE_PROPERTY_ID {
        StorageDeviceProperty = 0,
        StorageAdapterProperty,
        StorageDeviceIdProperty,
        StorageDeviceUniqueIdProperty,              // See storduid.h for details
        StorageDeviceWriteCacheProperty,
        StorageMiniportProperty,
        StorageAccessAlignmentProperty,
        StorageDeviceSeekPenaltyProperty,
        StorageDeviceTrimProperty,
        StorageDeviceWriteAggregationProperty,
        StorageDeviceDeviceTelemetryProperty,
        StorageDeviceLBProvisioningProperty,
        StorageDevicePowerProperty,
        StorageDeviceCopyOffloadProperty,
        StorageDeviceResiliencyProperty,
        StorageDeviceMediumProductType,
        StorageAdapterRpmbProperty,
        StorageAdapterCryptoProperty,
        StorageDeviceIoCapabilityProperty = 48,
        StorageAdapterProtocolSpecificProperty,
        StorageDeviceProtocolSpecificProperty,
        StorageAdapterTemperatureProperty,
        StorageDeviceTemperatureProperty,
        StorageAdapterPhysicalTopologyProperty,
        StorageDevicePhysicalTopologyProperty,
        StorageDeviceAttributesProperty,
        StorageDeviceManagementStatus,
        StorageAdapterSerialNumberProperty,
        StorageDeviceLocationProperty,
        StorageDeviceNumaProperty,
        StorageDeviceZonedDeviceProperty,
        StorageDeviceUnsafeShutdownCount
    } STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

    typedef struct _STORAGE_PROPERTY_QUERY {
        STORAGE_PROPERTY_ID PropertyId;
        STORAGE_QUERY_TYPE QueryType;
        BYTE  AdditionalParameters[1];
    } STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;

    typedef struct _PARTITION_INFORMATION_GPT {
        GUID PartitionType;                 // Partition type. See table 16-3.
        GUID PartitionId;                   // Unique GUID for this partition.
        ULONG64 Attributes;                 // See table 16-4.
        WCHAR Name [36];                    // Partition Name in Unicode.
    } PARTITION_INFORMATION_GPT, *PPARTITION_INFORMATION_GPT;

    typedef enum _PARTITION_STYLE {
        PARTITION_STYLE_MBR,
        PARTITION_STYLE_GPT,
        PARTITION_STYLE_RAW
    } PARTITION_STYLE;

    typedef SET_PARTITION_INFORMATION SET_PARTITION_INFORMATION_MBR;
    typedef PARTITION_INFORMATION_GPT SET_PARTITION_INFORMATION_GPT;

    typedef struct _SET_PARTITION_INFORMATION_EX {
        PARTITION_STYLE PartitionStyle;
        union {
            SET_PARTITION_INFORMATION_MBR Mbr;
            SET_PARTITION_INFORMATION_GPT Gpt;
        } DUMMYUNIONNAME;
    } SET_PARTITION_INFORMATION_EX, *PSET_PARTITION_INFORMATION_EX;

    typedef struct _PARTITION_INFORMATION_MBR {
        UCHAR PartitionType;
        BOOLEAN BootIndicator;
        BOOLEAN RecognizedPartition;
        ULONG HiddenSectors;
        GUID PartitionId;
    } PARTITION_INFORMATION_MBR, *PPARTITION_INFORMATION_MBR;

    typedef struct _PARTITION_INFORMATION_EX {
        PARTITION_STYLE PartitionStyle;
        LARGE_INTEGER StartingOffset;
        LARGE_INTEGER PartitionLength;
        ULONG PartitionNumber;
        BOOLEAN RewritePartition;

        union {
            PARTITION_INFORMATION_MBR Mbr;
            PARTITION_INFORMATION_GPT Gpt;
        } DUMMYUNIONNAME;
    } PARTITION_INFORMATION_EX, *PPARTITION_INFORMATION_EX;

    #define IOCTL_DISK_GET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0012, METHOD_BUFFERED, FILE_ANY_ACCESS)

    typedef struct _MEMORYSTATUSEX {
        DWORD     dwLength;
        DWORD     dwMemoryLoad;
        DWORDLONG ullTotalPhys;
        DWORDLONG ullAvailPhys;
        DWORDLONG ullTotalPageFile;
        DWORDLONG ullAvailPageFile;
        DWORDLONG ullTotalVirtual;
        DWORDLONG ullAvailVirtual;
        DWORDLONG ullAvailExtendedVirtual;
    } MEMORYSTATUSEX, *LPMEMORYSTATUSEX;

    typedef enum _LOGICAL_PROCESSOR_RELATIONSHIP {
      RelationProcessorCore,
      RelationNumaNode,
      RelationCache,
      RelationProcessorPackage,
      RelationGroup,
      RelationProcessorDie,
      RelationNumaNodeEx,
      RelationProcessorModule,
      RelationAll = 0xffff
    } LOGICAL_PROCESSOR_RELATIONSHIP;

    typedef enum _PROCESSOR_CACHE_TYPE {
      CacheUnified,
      CacheInstruction,
      CacheData,
      CacheTrace
    } PROCESSOR_CACHE_TYPE;

    typedef struct _CACHE_DESCRIPTOR {
      BYTE                 Level;
      BYTE                 Associativity;
      WORD                 LineSize;
      DWORD                Size;
      PROCESSOR_CACHE_TYPE Type;
    } CACHE_DESCRIPTOR, *PCACHE_DESCRIPTOR;

    typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION {
      ULONG_PTR                      ProcessorMask;
      LOGICAL_PROCESSOR_RELATIONSHIP Relationship;
      union {
        struct {
          BYTE Flags;
        } ProcessorCore;
        struct {
          DWORD NodeNumber;
        } NumaNode;
        CACHE_DESCRIPTOR Cache;
        ULONGLONG        Reserved[2];
      } DUMMYUNIONNAME;
    } SYSTEM_LOGICAL_PROCESSOR_INFORMATION, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;

    typedef ULONG_PTR KAFFINITY;

    typedef struct _GROUP_AFFINITY {
      KAFFINITY Mask;
      WORD      Group;
      WORD      Reserved[3];
    } GROUP_AFFINITY, *PGROUP_AFFINITY;

    typedef struct _CACHE_RELATIONSHIP {
      BYTE                 Level;
      BYTE                 Associativity;
      WORD                 LineSize;
      DWORD                CacheSize;
      PROCESSOR_CACHE_TYPE Type;
      BYTE                 Reserved[18];
      WORD                 GroupCount;
      union {
        GROUP_AFFINITY GroupMask;
        GROUP_AFFINITY GroupMasks[ANYSIZE_ARRAY];
      } DUMMYUNIONNAME;
    } CACHE_RELATIONSHIP, *PCACHE_RELATIONSHIP;

    typedef struct _PROCESSOR_GROUP_INFO {
      BYTE      MaximumProcessorCount;
      BYTE      ActiveProcessorCount;
      BYTE      Reserved[38];
      KAFFINITY ActiveProcessorMask;
    } PROCESSOR_GROUP_INFO, *PPROCESSOR_GROUP_INFO;

    typedef struct _GROUP_RELATIONSHIP {
      WORD                 MaximumGroupCount;
      WORD                 ActiveGroupCount;
      BYTE                 Reserved[20];
      PROCESSOR_GROUP_INFO GroupInfo[ANYSIZE_ARRAY];
    } GROUP_RELATIONSHIP, *PGROUP_RELATIONSHIP;

    typedef struct _NUMA_NODE_RELATIONSHIP {
      DWORD NodeNumber;
      BYTE  Reserved[18];
      WORD  GroupCount;
      union {
        GROUP_AFFINITY GroupMask;
        GROUP_AFFINITY GroupMasks[ANYSIZE_ARRAY];
      } DUMMYUNIONNAME;
    } NUMA_NODE_RELATIONSHIP, *PNUMA_NODE_RELATIONSHIP;

    typedef struct _PROCESSOR_RELATIONSHIP {
      BYTE           Flags;
      BYTE           EfficiencyClass;
      BYTE           Reserved[20];
      WORD           GroupCount;
      GROUP_AFFINITY GroupMask[ANYSIZE_ARRAY];
    } PROCESSOR_RELATIONSHIP, *PPROCESSOR_RELATIONSHIP;

    typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX {
      LOGICAL_PROCESSOR_RELATIONSHIP Relationship;
      DWORD                          Size;
      union {
        PROCESSOR_RELATIONSHIP Processor;
        NUMA_NODE_RELATIONSHIP NumaNode;
        CACHE_RELATIONSHIP     Cache;
        GROUP_RELATIONSHIP     Group;
      } DUMMYUNIONNAME;
    } SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX;

    typedef struct DXGI_ADAPTER_DESC
        {
        WCHAR Description[ 128 ];
        UINT VendorId;
        UINT DeviceId;
        UINT SubSysId;
        UINT Revision;
        SIZE_T DedicatedVideoMemory;
        SIZE_T DedicatedSystemMemory;
        SIZE_T SharedSystemMemory;
        LUID AdapterLuid;
        }   DXGI_ADAPTER_DESC;

    MIDL_INTERFACE("aec22fb8-76f3-4639-9be0-28eb43a67a2e")
    IDXGIObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPrivateData(
            REFGUID Name,
            UINT DataSize,
            const void *pData) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
            REFGUID Name,
            const IUnknown *pUnknown) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPrivateData(
            REFGUID Name,
            UINT *pDataSize,
            void *pData) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetParent(
            REFIID riid,
            void **ppParent) = 0;

    };

    MIDL_INTERFACE("2411e7e1-12ac-4ccf-bd14-9798e8534dc0")
    IDXGIAdapter : public IDXGIObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumOutputs(
            UINT Output,
            void **ppOutput) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetDesc(
            /* [annotation][out] */
            DXGI_ADAPTER_DESC *pDesc) = 0;

        virtual HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(
            REFGUID InterfaceName,
            LARGE_INTEGER *pUMDVersion) = 0;

    };

    MIDL_INTERFACE("7b7166ec-21c7-44ae-b21a-c9ae321ae369")
    IDXGIFactory : public IDXGIObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumAdapters(
            UINT Adapter,
            IDXGIAdapter **ppAdapter) = 0;
    };

    #define DXGI_ERROR_NOT_FOUND             _HRESULT_TYPEDEF_(0x887A0002L)

    typedef enum WHV_CAPABILITY_CODE
    {
        // Capabilities of the API implementation
        WHvCapabilityCodeHypervisorPresent      = 0x00000000,
        WHvCapabilityCodeFeatures               = 0x00000001,
        WHvCapabilityCodeExtendedVmExits        = 0x00000002,

        // Capabilities of the system's processor
        WHvCapabilityCodeProcessorVendor        = 0x00001000,
        WHvCapabilityCodeProcessorFeatures      = 0x00001001,
        WHvCapabilityCodeProcessorClFlushSize   = 0x00001002,
        WHvCapabilityCodeProcessorXsaveFeatures = 0x00001003,
    } WHV_CAPABILITY_CODE;

    typedef union WHV_CAPABILITY_FEATURES
    {
        struct
        {
            UINT64 PartialUnmap : 1;
            UINT64 LocalApicEmulation : 1;
            UINT64 Xsave : 1;
            UINT64 DirtyPageTracking : 1;
            UINT64 SpeculationControl : 1;
            UINT64 Reserved : 59;
        };

        UINT64 AsUINT64;
    } WHV_CAPABILITY_FEATURES;

    typedef enum WHV_PROCESSOR_VENDOR
    {
        WHvProcessorVendorAmd   = 0x0000,
        WHvProcessorVendorIntel = 0x0001,
        WHvProcessorVendorHygon = 0x0002

    } WHV_PROCESSOR_VENDOR;

    typedef union WHV_PROCESSOR_FEATURES
    {
        struct
        {
            /* CPUID.01H:ECX.SSE3[bit 0] = 1 */
            UINT64 Sse3Support : 1;
            /* CPUID.80000001H:ECX.LAHF-SAHF[bit 0] = 1 */
            UINT64 LahfSahfSupport : 1;
            /* CPUID.01H:ECX.SSSE3[bit 9] = 1 */
            UINT64 Ssse3Support : 1;
            /* CPUID.01H:ECX.SSE4_1[bit 19] = 1 */
            UINT64 Sse4_1Support : 1;
            /* CPUID.01H:ECX.SSE4_2[bit 20] = 1 */
            UINT64 Sse4_2Support : 1;
            /* CPUID.80000001H:ECX.SSE4A[bit 6] */
            UINT64 Sse4aSupport : 1;
            /* CPUID.80000001H:ECX.XOP[bit 11] */
            UINT64 XopSupport : 1;
            /* CPUID.01H:ECX.POPCNT[bit 23] = 1 */
            UINT64 PopCntSupport : 1;
            /* CPUID.01H:ECX.CMPXCHG16B[bit 13] = 1 */
            UINT64 Cmpxchg16bSupport : 1;
            /* CPUID.80000001H:ECX.AltMovCr8[bit 4] */
            UINT64 Altmovcr8Support : 1;
            /* CPUID.80000001H:ECX.LZCNT[bit 5] = 1 */
            UINT64 LzcntSupport : 1;
            /* CPUID.80000001H:ECX.MisAlignSse[bit 7] */
            UINT64 MisAlignSseSupport : 1;
            /* CPUID.80000001H:EDX.MmxExt[bit 22] */
            UINT64 MmxExtSupport : 1;
            /* CPUID.80000001H:EDX.3DNow[bit 31] */
            UINT64 Amd3DNowSupport : 1;
            /* CPUID.80000001H:EDX.3DNowExt[bit 30] */
            UINT64 ExtendedAmd3DNowSupport : 1;
            /* CPUID.80000001H:EDX.Page1GB[bit 26] = 1 */
            UINT64 Page1GbSupport : 1;
            /* CPUID.01H:ECX.AES[bit 25] */
            UINT64 AesSupport : 1;
            /* CPUID.01H:ECX.PCLMULQDQ[bit 1] = 1 */
            UINT64 PclmulqdqSupport : 1;
            /* CPUID.01H:ECX.PCID[bit 17] */
            UINT64 PcidSupport : 1;
            /* CPUID.80000001H:ECX.FMA4[bit 16] = 1 */
            UINT64 Fma4Support : 1;
            /* CPUID.01H:ECX.F16C[bit 29] = 1 */
            UINT64 F16CSupport : 1;
            /* CPUID.01H:ECX.RDRAND[bit 30] = 1 */
            UINT64 RdRandSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.FSGSBASE[bit 0] */
            UINT64 RdWrFsGsSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.SMEP[bit 7] */
            UINT64 SmepSupport : 1;
            /* IA32_MISC_ENABLE.FastStringsEnable[bit 0] = 1 */
            UINT64 EnhancedFastStringSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.BMI1[bit 3] = 1 */
            UINT64 Bmi1Support : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.BMI2[bit 8] = 1 */
            UINT64 Bmi2Support : 1;
            UINT64 Reserved1 : 2;
            /* CPUID.01H:ECX.MOVBE[bit 22] = 1 */
            UINT64 MovbeSupport : 1;
            UINT64 Reserved : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX[bit 13] = 1 */
            UINT64 DepX87FPUSaveSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.RDSEED[bit 18] = 1 */
            UINT64 RdSeedSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.ADX[bit 19] */
            UINT64 AdxSupport : 1;
            /* CPUID.80000001H:ECX.PREFETCHW[bit 8] = 1 */
            UINT64 IntelPrefetchSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.SMAP[bit 20] = 1 */
            UINT64 SmapSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.HLE[bit 4] = 1 */
            UINT64 HleSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.RTM[bit 11] = 1 */
            UINT64 RtmSupport : 1;
            /* CPUID.80000001H:EDX.RDTSCP[bit 27] = 1 */
            UINT64 RdtscpSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.CLFLUSHOPT[bit 23] */
            UINT64 ClflushoptSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.CLWB[bit 24] = 1 */
            UINT64 ClwbSupport : 1;
            /* CPUID.(EAX=07H, ECX=0H):EBX.SHA[bit 29] */
            UINT64 ShaSupport : 1;
            /* CPUID.80000008H:EBX[bit 2] = 1 (AMD only) */
            UINT64 X87PointersSavedSupport : 1;
            UINT64 InvpcidSupport : 1;
            UINT64 IbrsSupport : 1;
            UINT64 StibpSupport : 1;
            UINT64 IbpbSupport : 1;
            UINT64 Reserved2 : 1;
            UINT64 SsbdSupport : 1;
            UINT64 FastShortRepMovSupport : 1;
            UINT64 Reserved3 : 1;
            UINT64 RdclNo : 1;
            UINT64 IbrsAllSupport : 1;
            UINT64 Reserved4 : 1;
            UINT64 SsbNo : 1;
            UINT64 RsbANo : 1;
            UINT64 Reserved5 : 8;
        };

        UINT64 AsUINT64;
    } WHV_PROCESSOR_FEATURES;

    typedef union WHV_CAPABILITY
    {
        union
        {
            BOOL HypervisorPresent;
            WHV_CAPABILITY_FEATURES Features;
         //   WHV_EXTENDED_VM_EXITS ExtendedVmExits;
            WHV_PROCESSOR_VENDOR ProcessorVendor;
            WHV_PROCESSOR_FEATURES ProcessorFeatures;
         //   WHV_PROCESSOR_XSAVE_FEATURES ProcessorXsaveFeatures;
            char ProcessorClFlushSize;
        };
    } WHV_CAPABILITY;

#else
    #include <dxgi.h>
    #include <WinHvPlatform.h>
#endif // _MSC_VER_1200

const GUID GUID_IDXGIFactory = { 0x7b7166ec, 0x21c7, 0x44ae, { 0xb2, 0x1a, 0xc9, 0xae, 0x32, 0x1a, 0xe3, 0x69 } };

void PrintNumberWithCommas( char *pc, __int64 n )
{
    if ( n < 0 )
    {
        strcat( pc, "-" );
        PrintNumberWithCommas( pc, -n );
    }
    else if ( n < 1000 )
    {
        sprintf( pc + strlen( pc ), "%lld", n );
    }
    else
    {
        PrintNumberWithCommas( pc, n / 1000 );
        sprintf( pc + strlen( pc ), ",%03lld", n % 1000 );
    }
} //PrintNumberWithCommas

DWORD CountSetBits( ULONG_PTR bitMask )
{
    DWORD bitSetCount = 0;
    DWORD LSHIFT = sizeof( ULONG_PTR ) * 8 - 1;
    ULONG_PTR bitTest = (ULONG_PTR) 1 << LSHIFT;

    for ( DWORD i = 0; i <= LSHIFT; i++ )
    {
        bitSetCount += ( ( bitMask & bitTest ) ? 1 : 0 );
        bitTest /= 2;
    }

    return bitSetCount;
} //CountSetBits

const char * CacheTypeString( PROCESSOR_CACHE_TYPE t )
{
    if ( CacheUnified == t )
        return "unified";
    if ( CacheInstruction == t )
        return "instruction";
    if ( CacheData == t )
        return "data";
    if ( CacheTrace == t )
        return "trace";
    return "UNKNOWN";
} //CacheTypeString

struct EfficiencyAndMask
{
    BYTE efficiencyClass;
    KAFFINITY mask;
};

bool AttemptNewAPIForProcessorInfo()
{
    DWORD returnLength = 0;

    typedef BOOL ( WINAPI *LPFN_GLPIEX )( LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD );
    LPFN_GLPIEX glpi = (LPFN_GLPIEX) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetLogicalProcessorInformationEx" );
    if ( 0 == glpi )
    {
        printf( "GetLogicalProcessorInformationEx is not supported.\n" );
        return false;
    }

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = NULL;
    BOOL done = FALSE;
    do
    {
        DWORD rc = glpi( RelationAll, buffer, &returnLength );

        if ( FALSE == rc )
        {
            if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                if ( buffer )
                    free( buffer );

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) malloc( returnLength );
                memset( buffer, 0, returnLength );
            }
            else
            {
                printf( "\nError %d\n", GetLastError() );
                return false;
            }
        }
        else
        {
            done = TRUE;
        }
    } while ( !done );

    vector<CACHE_RELATIONSHIP> unique_cr;
    vector<DWORD> crcounts;
    vector<PROCESSOR_RELATIONSHIP> unique_pr;
    vector<DWORD> prcounts;

    BYTE maxEfficiency = 0;
    vector<EfficiencyAndMask> efficiencyAndMasks;

    bool shownL1 = false, shownL2 = false, shownL3 = false;
    DWORD numaNodeCount = 0;
    DWORD processorPackageCount = 0;
    DWORD logicalProcessorCount = 0;
    byte * pNext = (byte *) buffer;
    byte * beyond = pNext + returnLength;

    printf( "processor info via GetLogicalProcessorInformationEx:\n" );

    while ( pNext < beyond )
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) pNext;

        switch ( ptr->Relationship )
        {
            case RelationNumaNode:
            {
                numaNodeCount++;
                break;
            }
            case RelationProcessorCore:
            {
                PROCESSOR_RELATIONSHIP & pr = ptr->Processor;

                EfficiencyAndMask eam;
                eam.efficiencyClass = pr.EfficiencyClass;
                eam.mask = pr.GroupMask[ 0 ].Mask;
                efficiencyAndMasks.push_back( eam );
                if ( pr.EfficiencyClass > maxEfficiency )
                    maxEfficiency = pr.EfficiencyClass;

                bool found = false;
                for ( size_t i = 0; i < unique_pr.size(); i++ )
                {
                    PROCESSOR_RELATIONSHIP & p = unique_pr[ i ];

                    if ( pr.Flags == p.Flags &&
                         pr.EfficiencyClass == p.EfficiencyClass )
                    {
                        found = true;
                        prcounts[ i ]++;
                        break;
                    }
                }

                if ( !found )
                {
                    unique_pr.push_back( pr );
                    prcounts.push_back( 1 );
                }

                // A hyperthreaded core supplies more than one logical processor.

                logicalProcessorCount += ( LTP_PC_SMT == ptr->Processor.Flags ) ? 2 : 1;
                break;
            }
            case RelationCache:
            {
                // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.

                CACHE_RELATIONSHIP & cache = ptr->Cache;

                DWORD level = cache.Level;
                DWORD type = cache.Type;
                if ( level > 3 || type > 3 )
                {
                    printf( "(unexpected cache level and/or type: %d, %d\n", level, type );
                    break;
                }

                bool found = false;
                for ( size_t i = 0; i < unique_cr.size(); i++ )
                {
                    CACHE_RELATIONSHIP & c = unique_cr[ i ];

                    if ( c.Level == cache.Level &&
                         c.Associativity == cache.Associativity &&
                         c.LineSize == cache.LineSize &&
                         c.CacheSize == cache.CacheSize &&
                         c.Type == cache.Type )
                    {
                        found = true;
                        crcounts[ i ]++;
                        break;
                    }
                }

                if ( !found )
                {
                    unique_cr.push_back( cache );
                    crcounts.push_back( 1 );
                }

                break;
            }
            case RelationProcessorPackage:
            {
                // Logical processors share a physical package.
                processorPackageCount++;
                break;
            }
            case RelationGroup:
            {
                //printf( "processor group, maxgroupcount %d, active group count %d\n", ptr->Group.MaximumGroupCount, ptr->Group.ActiveGroupCount );
                break;
            }
            case 7:
            {
                // RelationProcessorModule (not in my headers yet)
                break;
            }
            default:
            {
                // 5 == RelationProcessorDie
                // 6 == RelationNumaNodeEx
                printf( "\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value: %#x\n", ptr->Relationship );
                break;
            }
        }

        pNext += ptr->Size;
    }

    printf( "  cache count / size / line size:\n" );
    for ( size_t i = 0; i < unique_cr.size(); i++ )
    {
        CACHE_RELATIONSHIP & cr = unique_cr[ i ];
        int typelen = strlen( CacheTypeString( cr.Type ) );

        printf(  "    L%d %s: %*s                      %d / %dK / %d\n",
                 cr.Level,
                 CacheTypeString( cr.Type ),
                 12 - typelen, " ",
                 crcounts[ i ],
                 cr.CacheSize / 1024,
                 cr.LineSize );
    }

    DWORD processorCount = 0;
    for ( size_t x = 0; x < unique_pr.size(); x++ )
    {
        PROCESSOR_RELATIONSHIP pr = unique_pr[ x ];
        printf( "  core count / efficiency / hyperthreads:  %d / %d%s / %s\n",
                prcounts[ x ],
                pr.EfficiencyClass,
                ( 0 == maxEfficiency ) ? "" :
                    ( 0 == pr.EfficiencyClass ) ? " (slower)" : " (faster)",
                0 == pr.Flags ? "no" : LTP_PC_SMT == pr.Flags ? "yes" : "unknown" );
        processorCount += prcounts[ x ];
    }

    printf( "  number of NUMA nodes:                    %d\n", numaNodeCount );
    printf( "  number of physical processor packages:   %d\n", processorPackageCount);
    printf( "  number of cores / processor units:       %d\n", processorCount );
    printf( "  number of logical processors:            %d\n", logicalProcessorCount );

    for ( BYTE e = 0; e <= maxEfficiency; e++ )
    {
        if ( 0 == maxEfficiency )
            printf( "  core hex masks at efficiency %d:         ", e );
        else
            printf( "  core hex masks at efficiency %d (%s):", e, ( 0 == e ) ? "slower" : "faster" );

        for ( size_t x = 0; x < efficiencyAndMasks.size(); x++ )
        {
            if ( e == efficiencyAndMasks[ x ].efficiencyClass )
                printf( " %llx", (ULONGLONG) efficiencyAndMasks[ x ].mask );
        }
        printf( "\n" );
    }

    free( buffer );

    return true;
} //AttemptNewAPIForProcessorInfo

bool AttemptOldAPIForProcessorInfo()
{
    DWORD returnLength = 0;
    typedef BOOL ( WINAPI *LPFN_GLPI )( PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD );
    LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetLogicalProcessorInformation" );
    if ( 0 == glpi )
    {
        printf( "GetLogicalProcessorInformation is not supported.\n" );
        return false;
    }

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    BOOL done = FALSE;
    do
    {
        DWORD rc = glpi( buffer, &returnLength );

        if ( FALSE == rc )
        {
            if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                if ( buffer )
                    free( buffer );

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) malloc( returnLength );
                memset( buffer, 0, returnLength );
            }
            else
            {
                printf( "\nError %d\n", GetLastError() );
                return false;
            }
        }
        else
        {
            done = TRUE;
        }
    } while ( !done );

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;
    bool shownL1 = false, shownL2 = false, shownL3 = false;

    while ( byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength )
    {
        switch (ptr->Relationship)
        {
            case RelationNumaNode:
            {
                numaNodeCount++;
                break;
            }
            case RelationProcessorCore:
            {
                processorCoreCount++;

                // A hyperthreaded core supplies more than one logical processor.

                logicalProcessorCount += CountSetBits( ptr->ProcessorMask );
                break;
            }
            case RelationCache:
            {
                // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.
                Cache = &ptr->Cache;
                if ( 1 == Cache->Level )
                {
                    if ( !shownL1 )
                    {
                        printf( "L1 cache / line size:                      %dk / %d\n", Cache->Size / 1024, Cache->LineSize );
                        shownL1 = true;
                    }
                    processorL1CacheCount++;
                }
                else if ( 2 == Cache->Level )
                {
                    if ( !shownL2 )
                    {
                        printf( "L2 cache / line size:                      %dk / %d\n", Cache->Size / 1024, Cache->LineSize );
                        shownL2 = true;
                    }
                    processorL2CacheCount++;
                }
                else if ( 3 == Cache->Level )
                {
                    if ( !shownL3 )
                    {
                        printf( "L3 cache / line size:                      %dk / %d\n", Cache->Size / 1024, Cache->LineSize );
                        shownL3 = true;
                    }
                    processorL3CacheCount++;
                }
                break;
            }
            case RelationProcessorPackage:
            {
                // Logical processors share a physical package.
                processorPackageCount++;
                break;
            }
            default:
            {
                printf( "\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n" );
                break;
            }
        }

        byteOffset += sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION );
        ptr++;
    }

    printf( "processor info via GetLogicalProcessorInformation:\n" );
    printf( "  number of processor L1 / L2 / L3 caches: %d / %d / %d\n", processorL1CacheCount, processorL2CacheCount, processorL3CacheCount );
    printf( "  number of NUMA nodes:                    %d\n", numaNodeCount );
    printf( "  number of physical processor packages:   %d\n", processorPackageCount);
    printf( "  number of cores / processor units:       %d\n", processorCoreCount );
    printf( "  number of logical processors:            %d\n", logicalProcessorCount );

    free( buffer );

    return true;
} //AttemptOldAPIForProcessorInfo

void ShowSystemMemory()
{
    bool bannerShown = false;
    typedef BOOL ( WINAPI *LPFN_GPISM )( PULONGLONG );
    LPFN_GPISM gpism = (LPFN_GPISM) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetPhysicallyInstalledSystemMemory" );
    if ( 0 != gpism )
    {
        ULONGLONG meminK;
        BOOL worked = gpism( &meminK );
        if ( worked )
        {
            if ( !bannerShown )
            {
                printf( "system memory info via GetPhysicallyInstalledSystemMemory:\n" );
                bannerShown = true;
            }
            const ULONGLONG oneK = 1024;
            ULONGLONG meminMeg = meminK / oneK;
            ULONGLONG meminGig = meminMeg / oneK;

            // vs6 printf can't print two 64 bit integers in one format string
            printf( "  physically installed system memory:      %lld MB == ", meminMeg );
            printf( "%lld GB\n", meminGig );
        }
    }

    typedef BOOL ( WINAPI *LPFN_GMSEX ) ( LPMEMORYSTATUSEX );
    LPFN_GMSEX gmsex = (LPFN_GMSEX) GetProcAddress( GetModuleHandleA( "kernel32" ), "GlobalMemoryStatusEx" );
    if ( 0 != gmsex )
    {
        MEMORYSTATUSEX msex;
        msex.dwLength = sizeof( msex );
        BOOL ok = gmsex( &msex );
        if ( ok )
        {
            if ( !bannerShown )
            {
                printf( "system memory info via GlobalMemoryStatusEx:\n" );
                bannerShown = true;
            }
            const ULONGLONG oneK = 1024;
            ULONGLONG meminK = msex.ullTotalPhys / oneK;
            ULONGLONG meminMeg = meminK / oneK;
            double meminGig = (double) (LONGLONG) meminK / (double) OneMeg;

            // vs6 printf can't print two 64 bit integers in one format string
            printf( "  total usable physical memory:            %lld MB == ", meminMeg );
            printf( "%.2lf GB\n", meminGig );
        }
        else
            printf( "GlobalMemoryStatuEx failed with error %d\n", GetLastError() );
    }
    else
    {
        typedef void ( WINAPI *LPFN_GMS ) ( LPMEMORYSTATUS );
        LPFN_GMS gms = (LPFN_GMS) GetProcAddress( GetModuleHandleA( "kernel32" ), "GlobalMemoryStatus" );
        if ( 0 != gms )
        {
            if ( !bannerShown )
            {
                printf( "system memory info via GlobalMemoryStatus:\n" );
                bannerShown = true;
            }
            MEMORYSTATUS ms;
            ms.dwLength = sizeof( ms );
            gms( &ms );
            printf( "  total usable physical memory:            %llu bytes\n", (ULONGLONG) ms.dwTotalPhys );
        }
        else
            printf( "unable to find GlobalMemoryStatus in kernel32, error %d\n", GetLastError() );
    }
} //ShowSystemMemory

void PartitionSize( char c )
{
    char acFile[10];
    strcpy( acFile, "\\\\.\\c:" );
    acFile[4] = c;
    BOOL fOk = FALSE;

    HANDLE hDevice = CreateFileA( acFile, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if ( INVALID_HANDLE_VALUE != hDevice )
    {
        DWORD dwRet = 0;
        PARTITION_INFORMATION pi;
        PARTITION_INFORMATION_EX piex;
        char ac[ 100 ] = {0};

        if ( DeviceIoControl( hDevice, IOCTL_DISK_GET_PARTITION_INFO_EX, 0, 0, &piex, sizeof( piex ), &dwRet, 0 ) )
        {
            _int64 len = piex.PartitionLength.QuadPart;
            ac[ 0 ] = 0;
            PrintNumberWithCommas( ac, len / OneMeg );
            printf( " %14sm                ", ac );
            fOk = TRUE;
        }
        else if ( DeviceIoControl( hDevice, IOCTL_DISK_GET_PARTITION_INFO, 0, 0, &pi, sizeof( pi ), &dwRet, 0 ) )
        {
            _int64 len = pi.PartitionLength.LowPart;
            _int64 hi = pi.PartitionLength.HighPart;
            len += ( hi << 32 );
            ac[ 0 ] = 0;
            PrintNumberWithCommas( ac, len / OneMeg );
            printf( " %14sm                ", ac );
            fOk = TRUE;
        }

        CloseHandle( hDevice );
    }

    if ( !fOk )
        printf( "                                " );
} //PartitionSize

bool GetDriveInformation( char * pcDriveName, bool & supportsTrim, bool & seekPenalty )
{
    supportsTrim = false;
    seekPenalty = true;
    bool success = false;

    char acFile[ 10 ];
    strcpy(acFile,"\\\\.\\c:");
    acFile[4] = *pcDriveName;

    HANDLE hDevice = CreateFileA( acFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );
    if ( INVALID_HANDLE_VALUE == hDevice )
        return false;

    STORAGE_PROPERTY_QUERY spqTrim;
    spqTrim.PropertyId = StorageDeviceTrimProperty;
    spqTrim.QueryType = PropertyStandardQuery;

    DWORD bytesReturned = 0;
    DEVICE_TRIM_DESCRIPTOR dtd = {0};
    if ( DeviceIoControl( hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &spqTrim, sizeof(spqTrim), &dtd, sizeof( dtd ), &bytesReturned, 0 ) &&
         bytesReturned == sizeof( dtd ) )
    {
        supportsTrim = ( 0 != dtd.TrimEnabled ); // true for SSDs that support trim and aren't in a raid volume
        success = true;
    }

    STORAGE_PROPERTY_QUERY spqSeekP;
    spqSeekP.PropertyId = StorageDeviceSeekPenaltyProperty;
    spqSeekP.QueryType = PropertyStandardQuery;

    bytesReturned = 0;
    DEVICE_SEEK_PENALTY_DESCRIPTOR dspd = {0};
    if ( DeviceIoControl( hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &spqSeekP, sizeof( spqSeekP ), &dspd, sizeof( dspd ), &bytesReturned, 0 ) &&
         bytesReturned == sizeof( dspd ) )
    {
        seekPenalty = ( 0 != dspd.IncursSeekPenalty ); // true for spinning and false for SSD
        success = true;
    }

    CloseHandle( hDevice );
    return success;
} //GetDriveInformation

VOID VolumeInfo( char * pcDriveName )
{
    char acVolume[50], acFS[50];

    if ( GetVolumeInformationA( pcDriveName, acVolume, sizeof( acVolume ), 0, 0, 0, acFS, sizeof( acFS ) ) )
        printf( "  %5s %24s", acFS, acVolume );
    else
        printf( "      ?                                                         " );

    DWORD dwSPerC,dwBPerS,dwFreeClusters,dwClusters;

    char ac[ 100 ];
    typedef BOOL ( WINAPI *LPFN_GDFSEX ) ( const char *, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER );
    LPFN_GDFSEX gdfsex = (LPFN_GDFSEX) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetDiskFreeSpaceExA" );
    if ( 0 != gdfsex )
    {
        ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
        BOOL ok = gdfsex( pcDriveName, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes );
        if ( ok )
        {
            ac[ 0 ] = 0;
            PrintNumberWithCommas( ac, totalNumberOfBytes.QuadPart / OneMeg  );
            printf( " %14sm", ac );

            ac[ 0 ] = 0;
            PrintNumberWithCommas( ac, freeBytesAvailable.QuadPart / OneMeg );
            printf( " %14sm", ac );
        }
    }
    else if ( GetDiskFreeSpaceA( pcDriveName, &dwSPerC, &dwBPerS, &dwFreeClusters, &dwClusters ) )
    {
        _int64 spc = dwSPerC;
        _int64 sbps = dwBPerS;
        _int64 fc = dwFreeClusters;
        _int64 c = dwClusters;

        ac[ 0 ] = 0;
        PrintNumberWithCommas( ac, ( spc * sbps * c ) / OneMeg );
        printf( " %14sm", ac );

        ac[ 0 ] = 0;
        PrintNumberWithCommas( ac, ( spc * sbps * fc ) / OneMeg );
        printf( " %14sm", ac );
    }
    else
    {
        PartitionSize( pcDriveName[0] );
    }

    bool supportsTrim = false;
    bool seekPenalty = true;
    bool ok = GetDriveInformation( pcDriveName, supportsTrim, seekPenalty );
    if ( ok )
        printf( "%10s", supportsTrim ? "yes" : "no" );
} //VolumeInfo

void ShowDrives()
{
    printf( "drive info via GetDriveType, GetDiskFreeSpace(Ex), IOCTL_DISK_GET_PARTITION_INFO(_EX), IOCTL_STORAGE_QUERY_PROPERTY:\n" );
    printf("      type      fs                   volume           total            free      trim\n");

    DWORD dwDriveMask = GetLogicalDrives();
    char acRootPath[ 5 ];
    strcpy( acRootPath, "?:\\" );

    for ( acRootPath[0] = 'a'; acRootPath[0] <= 'z'; acRootPath[0]++ )
    {
        DWORD dwExists = dwDriveMask & 1;
        dwDriveMask >>= 1;

        if ( 0 != dwExists )
        {
            DWORD driveType = GetDriveTypeA( acRootPath );
            printf( "  %c:  ", acRootPath[0] );

            switch( driveType )
            {
                case DRIVE_REMOVABLE:
                {
                    printf( "remov" );

                    SetErrorMode( 1 ); // no popups if no disk in drive!

                    if ( acRootPath[ 0 ] != 'a' && acRootPath[ 0 ] != 'b' )
                        VolumeInfo( acRootPath );
                    else
                        printf("      -");

                    SetErrorMode( 0 );
                    break;
                }
                case DRIVE_FIXED:
                {
                    printf( "fixed" );
                    VolumeInfo( acRootPath );
                    break;
                }
                case DRIVE_REMOTE:
                {
                    printf( "net  " );
                    VolumeInfo( acRootPath );
                    break;
                }
                case DRIVE_CDROM:
                {
                    printf( "cdrom" );
                    SetErrorMode(1); // no popups if no disk in drive!
                    VolumeInfo( acRootPath );
                    SetErrorMode(0);
                    break;
                }
                case DRIVE_RAMDISK:
                {
                    printf( "ram  " );
                    VolumeInfo( acRootPath );
                    break;
                }
                default:
                {
                    printf("    ?");
                    break;
                }
            }

            printf("\n");
        }
    }
} //ShowDrives

#define pm( x ) ( x ? '+' : '-' )

bool ShowHypervisorInfo()
{
    typedef HRESULT ( WINAPI *LPFN_WHGC )( WHV_CAPABILITY_CODE, PVOID, UINT32, UINT32 * );
    HMODULE hmod = LoadLibraryA( "WinHvPlatform.dll" );
    if ( 0 == hmod )
    {
        //printf( "\nCan't LoadLibrary WinHvPlatform.dll, error %d.\n", GetLastError() );
        return false;
    }

    LPFN_WHGC whgc = (LPFN_WHGC) GetProcAddress( hmod, "WHvGetCapability" );
    if ( 0 == whgc )
    {
        printf( "\nWHvGetCapability is not supported, error %d.\n", GetLastError() );
        FreeLibrary( hmod );
        return false;
    }

    WHV_CAPABILITY cap = { 0 };
    UINT32 cbWritten = 0;
    HRESULT hr = whgc( WHvCapabilityCodeHypervisorPresent, & cap, sizeof( cap ), & cbWritten );
    if ( S_OK == hr )
    {
        printf( "hypervisor information via WHvGetCapability:\n" );
        printf( "  hypervisor present:                      %s\n", cap.HypervisorPresent ? "true" : "false" );

        if ( !cap.HypervisorPresent )
            return true;
    }
    else
    {
        printf( "WHvGetCapability WHvCapabilityCodeHypervisorPresent failed with error %#x\n", hr );
        FreeLibrary( hmod );
        return false;
    }

    cbWritten = 0;
    hr = whgc( WHvCapabilityCodeFeatures, & cap, sizeof( cap ), & cbWritten );
    if ( S_OK == hr )
    {
        printf( "  partial unmap:                           %s\n", cap.Features.PartialUnmap ? "true" : "false" );
        printf( "  local apic emulation:                    %s\n", cap.Features.LocalApicEmulation ? "true" : "false" );
        printf( "  xsave:                                   %s\n", cap.Features.Xsave ? "true" : "false" );
        printf( "  dirty page tracking:                     %s\n", cap.Features.DirtyPageTracking ? "true" : "false" );
        printf( "  speculation control:                     %s\n", cap.Features.SpeculationControl ? "true" : "false" );
    }
    else
    {
        printf( "WHvGetCapability WHvCapabilityCodeFeatures failed with error %#x\n", hr );
        FreeLibrary( hmod );
        return false;
    }

    cbWritten = 0;
    hr = whgc( WHvCapabilityCodeProcessorVendor, & cap, sizeof( cap ), & cbWritten );
    if ( S_OK == hr )
    {
        UINT64 v = cap.ProcessorVendor;
        printf( "  processor vendor:                        %llu == %s\n", v, ( 0 == v ) ? "AMD" : ( 1 == v ) ? "Intel" : ( 2 == v ) ? "Hygon" : "unknown" );
    }
    else
    {
        printf( "WHvGetCapability WHvCapabilityCodeProcessorVendor failed with error %#x\n", hr );
        FreeLibrary( hmod );
        return false;
    }

    cbWritten = 0;
    hr = whgc( WHvCapabilityCodeProcessorFeatures, & cap, sizeof( cap ), & cbWritten );
    if ( S_OK == hr )
    {
        WHV_PROCESSOR_FEATURES f = cap.ProcessorFeatures;
        printf( "  processor features:                      " );
        printf( "Sse3%c, LahfSahf%c, Ssse3%c, Sse4_1%c, Sse4_2%c, Sse4a%c, Xop%c, PopCnt%c\n",
                pm( f.Sse3Support ), pm( f.LahfSahfSupport ), pm( f.Ssse3Support ), pm( f.Sse4_1Support ), pm( f.Sse4_2Support ), pm( f.Sse4aSupport ), pm( f.XopSupport ), pm( f.PopCntSupport ) );
        printf( "                                           " );
        printf( "Cmpxchg16b%c, Altmovcr8%c, Lzcnt%c, MisAlignSse%c, MmxExt%c, Amd3DNow%c, ExtendedAmd3DNow%c, Page1GB%c, Aes%c\n",
                pm( f.Cmpxchg16bSupport ), pm( f.Altmovcr8Support ), pm( f.LzcntSupport ), pm( f.MisAlignSseSupport ),
                pm( f.MmxExtSupport ), pm( f.Amd3DNowSupport ), pm( f.ExtendedAmd3DNowSupport ), pm( f.Page1GbSupport ), pm( f.AesSupport ) );
        printf( "                                           " );
        printf( "Pclmulqdq%c, Pcid%c, Fma4%c, F16C%c, RdRand%c, RdWrFsGs%c, Smep%c, EnhancedFastString%c, Bmi1%c, Bmi2%c\n",
                pm( f.PclmulqdqSupport ), pm( f.PcidSupport ), pm( f.Fma4Support ), pm( f.F16CSupport ), pm( f.RdRandSupport ),
                pm( f.RdWrFsGsSupport ), pm( f.SmepSupport ), pm( f.EnhancedFastStringSupport ), pm( f.Bmi1Support ), pm( f.Bmi2Support ) );
        printf( "                                           " );
        printf( "Movbe%c, DepX87FPUSave%c, RdSeed%c, Adx%c, IntelPrefetch%c, Smap%c, Hle%c, Rtm%c, Rdtscp%c, Clflushopt%c, Clwb%c, Sha%c\n",
                pm( f.MovbeSupport ), pm( f.DepX87FPUSaveSupport ), pm( f.RdSeedSupport ), pm( f.AdxSupport ), pm( f.IntelPrefetchSupport ), pm( f.SmapSupport ),
                pm( f.HleSupport ), pm( f.RtmSupport ), pm( f.RdtscpSupport ), pm( f.ClflushoptSupport ), pm( f.ClwbSupport ), pm( f.ShaSupport ) );
    }
    else
    {
        printf( "WHvGetCapability WHvCapabilityCodeProcessorVendor failed with error %#x\n", hr );
        FreeLibrary( hmod );
        return false;
    }

    return true;
} //ShowHypervisorInfo

bool ShowCPUSpeed()
{
    typedef HRESULT ( WINAPI *LPFN_CNTPI )( POWER_INFORMATION_LEVEL, PVOID, ULONG, PVOID, ULONG );
    HMODULE hmod = LoadLibraryA( "powrprof.dll" );
    if ( 0 == hmod )
    {
        printf( "\nCan't LoadLibrary powrprof.dll, error %d.\n", GetLastError() );
        return false;
    }

    LPFN_CNTPI cntpi = (LPFN_CNTPI) GetProcAddress( hmod, "CallNtPowerInformation" );
    if ( 0 == cntpi )
    {
        printf( "\nCallNtPowerInformation is not supported, error %d.\n", GetLastError() );
        FreeLibrary( hmod );
        return false;
    }

    SYSTEM_INFO si;
    GetSystemInfo( &si );
    DWORD bufferSize = si.dwNumberOfProcessors * sizeof( PROCESSOR_POWER_INFORMATION_FAKE );
    PROCESSOR_POWER_INFORMATION_FAKE * ppi = (PROCESSOR_POWER_INFORMATION_FAKE *) malloc( bufferSize );
    HRESULT hr = cntpi( ProcessorInformation, 0, 0, ppi, bufferSize );
    if ( S_OK == hr )
    {
        printf( "cpu speed via CallNtPowerInformation:\n" );

        // Note: on the AMD 5950x all three of the values are identical. On Intel CPUs these sometimes work.
        // Just show the first one.

        for ( DWORD i = 0; i < 1; i++ )
        {
            printf( "  Mhz current:                             %d\n", ppi[i].CurrentMhz );
            printf( "  Mhz max:                                 %d\n", ppi[i].MaxMhz );
            printf( "  Mhz limit:                               %d\n", ppi[i].MhzLimit );
        }
    }
    else
    {
        printf( "CallNtPowerInformation failed with error %#x\n", hr );
        free( ppi );
        FreeLibrary( hmod );
        return false;
    }

    free( ppi );
    FreeLibrary( hmod );
    return true;
} //ShowCPUSpeed

void PrintSystemInfo( SYSTEM_INFO & si )
{
    WORD arch = si.wProcessorArchitecture;
    printf( "  processor architecture:                  " );

    if ( PROCESSOR_ARCHITECTURE_INTEL == arch )
        printf( "x86\n" );
    else if ( PROCESSOR_ARCHITECTURE_AMD64 == arch )
        printf( "x64\n" );
    else if ( PROCESSOR_ARCHITECTURE_ARM == arch )
        printf( "arm\n" );
    else if ( PROCESSOR_ARCHITECTURE_ARM64 == arch )
        printf( "arm64\n" );
    else if ( PROCESSOR_ARCHITECTURE_IA64 == arch )
        printf( "ia64\n" );
    else
        printf( "unknown: %d\n", arch );

    printf( "  page size:                               %u\n", si.dwPageSize );
    printf( "  allocation granularity:                  %u\n", si.dwAllocationGranularity );
    printf( "  processor level (family):                %u\n", si.wProcessorLevel );
    printf( "  processor revision (model):              %#x\n", si.wProcessorRevision );
    printf( "  number of processors:                    %u\n", si.dwNumberOfProcessors );
} //PrintSystemInfo

void ShowSystemInfo()
{
    SYSTEM_INFO si;
    GetSystemInfo( &si );

    printf( "system info via GetSystemInfo:\n" );
    PrintSystemInfo( si );

    typedef void ( WINAPI *LPFN_GNSI ) ( LPSYSTEM_INFO );
    LPFN_GNSI gnsi = (LPFN_GNSI) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetNativeSystemInfo" );
    if ( 0 != gnsi )
    {
        SYSTEM_INFO native_si;
        gnsi( &native_si );
        if ( memcmp( &si, &native_si, sizeof si ) )
        {
            printf( "native system info via GetNativeSystemInfo:\n" );
            PrintSystemInfo( native_si );
        }
    }
} //ShowSystemInfo

void ShowNetworkAdapters()
{
    typedef ULONG ( WINAPI *LPFN_GAI ) ( PIP_ADAPTER_INFO, PULONG );
    HMODULE hmod = LoadLibraryA( "iphlpapi.dll" );
    if ( 0 == hmod )
    {
        printf( "Unable to load iphlpapi.dll for network information\n" );
        return;
    }

    LPFN_GAI gai = (LPFN_GAI) GetProcAddress( hmod, "GetAdaptersInfo" );
    if ( 0 == gai )
    {
        printf( "Unable to load find GetAdaptersInfo for network information, error %d\n", GetLastError() );
        FreeLibrary( hmod );
        return;
    }

    ULONG outBufLen = 0;
    IP_ADAPTER_INFO * pAdapterInfos = 0;

    do
    {
        DWORD dwRetVal = gai( pAdapterInfos, & outBufLen );
        if ( NO_ERROR == dwRetVal )
            break;

        free( pAdapterInfos );

        if ( ERROR_BUFFER_OVERFLOW == dwRetVal )
        {
            pAdapterInfos = (IP_ADAPTER_INFO *) malloc( outBufLen );
        }
        else
        {
            pAdapterInfos = 0;
            break;
        }
    } while( true );

    if ( pAdapterInfos )
    {
        printf( "network adapters via GetAdaptersInfo:\n" );
        IP_ADAPTER_INFO * pAdapter = pAdapterInfos;

        do
        {
            printf( "  adapter description:                     %s\n", pAdapter->Description );
            printf( "    adapter name:                          %s\n", pAdapter->AdapterName );
            printf( "    ipaddr / subnet mask / gateway / dhcp: %s / %s / %s / %s\n", pAdapter->IpAddressList.IpAddress.String,
                                                                                      pAdapter->IpAddressList.IpMask.String,
                                                                                      pAdapter->GatewayList.IpAddress.String,
                                                                                      pAdapter->DhcpEnabled ? "true" : "false" );

            printf( "    adapter (mac) address:                 " );
            for ( int i = 0; i < pAdapter->AddressLength; i++ )
            {
                printf( "%.2X", (int) pAdapter->Address[i] );
                if ( i != ( pAdapter->AddressLength - 1 ) )
                    printf( "-" );
            }
            printf( "\n" );

            pAdapter = pAdapter->Next;
        } while ( 0 != pAdapter );
    }

    FreeLibrary( hmod );
    free( pAdapterInfos );
} //ShowNetworkAdapters

void ShowNames()
{
    char acBuffer[ 100 ];

    typedef BOOL ( WINAPI *LPFN_GCNA ) ( char *, LPDWORD );
    LPFN_GCNA gcna = (LPFN_GCNA) GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetComputerNameA" );

    if ( gcna )
    {
        DWORD len = sizeof( acBuffer );
        BOOL ok = gcna( acBuffer, &len );
        if ( ok )
            printf( "computer name:                             %s\n", acBuffer );
    }
    else
        printf( "unable to get computer name\n" );

    HMODULE hmodAdvApi = LoadLibraryA( "advapi32.dll" );
    if ( 0 == hmodAdvApi )
    {
        printf( "\nCan't LoadLibrary advapi32.dll, error %d.\n", GetLastError() );
        return;
    }

    typedef BOOL ( WINAPI *LPFN_GUNA ) ( char *, LPDWORD );
    LPFN_GUNA guna = (LPFN_GUNA) GetProcAddress( hmodAdvApi, "GetUserNameA" );

    if ( guna )
    {
        DWORD len = sizeof( acBuffer );
        BOOL ok = guna( acBuffer, &len );
        if ( ok )
            printf( "user name:                                 %s\n", acBuffer );
    }
    else
        printf( "unable to get user name\n" );

    FreeLibrary( hmodAdvApi );
} //ShowNames

struct OSVerInfo
{
    DWORD id;
    const char * name;
};

OSVerInfo const osVersions[] =
{
    { 0x00000006, "Business", },
    { 0x00000010, "Business N", },
    { 0x00000012, "HPC Edition", },
    { 0x00000040, "Server Hyper Core V", },
    { 0x00000065, "Windows 10 Home", },
    { 0x00000063, "Windows 10 Home China", },
    { 0x00000062, "Windows 10 Home N", },
    { 0x00000064, "Windows 10 Home Single Language", },
    { 0x00000050, "Server Datacenter (evaluation installation)", },
    { 0x00000091, "Server Datacenter, Semi-Annual Channel (core installation)", },
    { 0x00000092, "Server Standard, Semi-Annual Channel (core installation)", },
    { 0x00000008, "Server Datacenter (full installation)", },
    { 0x0000000C, "Server Datacenter (core installation, Windows Server 2008 R2 and earlier)", },
    { 0x00000027, "Server Datacenter without Hyper-V (core installation)", },
    { 0x00000025, "Server Datacenter without Hyper-V (full installation)", },
    { 0x00000079, "Windows 10 Education", },
    { 0x0000007A, "Windows 10 Education N", },
    { 0x00000004, "Windows 10 Enterprise", },
    { 0x00000046, "Windows 10 Enterprise E", },
    { 0x00000048, "Windows 10 Enterprise Evaluation", },
    { 0x0000001B, "Windows 10 Enterprise N", },
    { 0x00000054, "Windows 10 Enterprise N Evaluation", },
    { 0x0000007D, "Windows 10 Enterprise 2015 LTSB", },
    { 0x00000081, "Windows 10 Enterprise 2015 LTSB Evaluation", },
    { 0x0000007E, "Windows 10 Enterprise 2015 LTSB N", },
    { 0x00000082, "Windows 10 Enterprise 2015 LTSB N Evaluation", },
    { 0x0000000A, "Server Enterprise (full installation)", },
    { 0x0000000E, "Server Enterprise (core installation)", },
    { 0x00000029, "Server Enterprise without Hyper-V (core installation)", },
    { 0x0000000F, "Server Enterprise for Itanium-based Systems", },
    { 0x00000026, "Server Enterprise without Hyper-V (full installation)", },
    { 0x0000003C, "Windows Essential Server Solution Additional", },
    { 0x0000003E, "Windows Essential Server Solution Additional SVC", },
    { 0x0000003B, "Windows Essential Server Solution Management", },
    { 0x0000003D, "Windows Essential Server Solution Management SVC", },
    { 0x00000002, "Home Basic", },
    { 0x00000043, "Not supported", },
    { 0x00000005, "Home Basic N", },
    { 0x00000003, "Home Premium", },
    { 0x00000044, "Not supported", },
    { 0x0000001A, "Home Premium N", },
    { 0x00000022, "Windows Home Server 2011", },
    { 0x00000013, "Windows Storage Server 2008 R2 Essentials", },
    { 0x0000002A, "Microsoft Hyper-V Server", },
    { 0x000000BC, "Windows IoT Enterprise", },
    { 0x000000BF, "Windows IoT Enterprise LTSC", },
    { 0x0000007B, "Windows 10 IoT Core", },
    { 0x00000083, "Windows 10 IoT Core Commercial", },
    { 0x0000001E, "Windows Essential Business Server Management Server", },
    { 0x00000020, "Windows Essential Business Server Messaging Server", },
    { 0x0000001F, "Windows Essential Business Server Security Server", },
    { 0x00000068, "Windows 10 Mobile", },
    { 0x00000085, "Windows 10 Mobile Enterprise", },
    { 0x0000004D, "Windows MultiPoint Server Premium (full installation)", },
    { 0x0000004C, "Windows MultiPoint Server Standard (full installation)", },
    { 0x000000A1, "Windows 10 Pro for Workstations", },
    { 0x000000A2, "Windows 10 Pro for Workstations N", },
    { 0x00000030, "Windows 10 Pro", },
    { 0x00000045, "Not supported", },
    { 0x00000031, "Windows 10 Pro N", },
    { 0x00000067, "Professional with Media Center", },
    { 0x00000032, "Windows Small Business Server 2011 Essentials", },
    { 0x00000036, "Server For SB Solutions EM", },
    { 0x00000033, "Server For SB Solutions", },
    { 0x00000037, "Server For SB Solutions EM", },
    { 0x00000018, "Windows Server 2008 for Windows Essential Server Solutions", },
    { 0x00000023, "Windows Server 2008 without Hyper-V for Windows Essential Server Solutions", },
    { 0x00000021, "Server Foundation", },
    { 0x00000009, "Windows Small Business Server", },
    { 0x00000019, "Small Business Server Premium", },
    { 0x0000003F, "Small Business Server Premium (core installation)", },
    { 0x00000038, "Windows MultiPoint Server", },
    { 0x0000004F, "Server Standard (evaluation installation)", },
    { 0x00000007, "Server Standard (full installation)", },
    { 0x0000000D, "Server Standard (core installation, Windows Server 2008 R2 and earlier)", },
    { 0x00000028, "Server Standard without Hyper-V (core installation)", },
    { 0x00000024, "Server Standard without Hyper-V", },
    { 0x00000034, "Server Solutions Premium", },
    { 0x00000035, "Server Solutions Premium (core installation)", },
    { 0x0000000B, "Starter", },
    { 0x00000042, "Not supported", },
    { 0x0000002F, "Starter N", },
    { 0x00000017, "Storage Server Enterprise", },
    { 0x0000002E, "Storage Server Enterprise (core installation)", },
    { 0x00000014, "Storage Server Express", },
    { 0x0000002B, "Storage Server Express (core installation)", },
    { 0x00000060, "Storage Server Standard (evaluation installation)", },
    { 0x00000015, "Storage Server Standard", },
    { 0x0000002C, "Storage Server Standard (core installation)", },
    { 0x0000005F, "Storage Server Workgroup (evaluation installation)", },
    { 0x00000016, "Storage Server Workgroup", },
    { 0x0000002D, "Storage Server Workgroup (core installation)", },
    { 0x00000001, "Ultimate", },
    { 0x00000047, "Not supported", },
    { 0x0000001C, "Ultimate N", },
    { 0x00000000, "An unknown product", },
    { 0x00000011, "Web Server (full installation)", },
    { 0x0000001D, "Web Server ", },
};

const char * OSVersionString( DWORD id )
{
    for ( int i = 0; i < _countof( osVersions ); i++ )
    {
        if ( id == osVersions[ i ].id )
            return osVersions[ i ].name;
    }

    return "unknown";
} //OSVersionString

void ShowOSVersion()
{
    bool gotVersionInfo = false;
    OSVERSIONINFOEXW ver;
    ver.dwOSVersionInfoSize = sizeof ver;

    // this app has no manifest, so Windows will lie to it about version information. Go to Rtl instead if available.

    typedef NTSTATUS ( WINAPI *LPFN_RGV ) ( LPOSVERSIONINFOEXW );
    HMODULE hmodNtdll = GetModuleHandleA( "ntdll.dll" );
    if ( hmodNtdll )
    {
        LPFN_RGV rgv = (LPFN_RGV) GetProcAddress( hmodNtdll, "RtlGetVersion" );
        if ( rgv )
        {
            NTSTATUS s = rgv( &ver );
            if ( 0 == s )
            {
                printf( "os version via RtlGetVersion:\n" );
                gotVersionInfo = true;
            }
        }
    }

    if ( !gotVersionInfo )
    {
        typedef BOOL ( WINAPI *LPFN_GVEA ) ( LPOSVERSIONINFOEXW );
        LPFN_GVEA gvea = (LPFN_GVEA) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetVersionExW" );
        if ( gvea )
        {
            BOOL ok = gvea( &ver );
            if ( ok )
            {
                printf( "os version via GetVersionEx:\n" );
                gotVersionInfo = true;
            }
        }
    }

    if ( gotVersionInfo )
    {
        printf( "  major.minor.build:                       %d.%d.%d\n", ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber );
        if ( ver.dwBuildNumber > 22000 )
            printf( "  build # > 22000, so it's at least        Windows 11\n" );
        if ( 0 != ver.szCSDVersion[ 0 ] )
            printf( "  service pack:                            %ws\n", ver.szCSDVersion );

        typedef BOOL ( WINAPI *LPFN_GPI ) ( DWORD, DWORD, DWORD, DWORD, PDWORD );
        LPFN_GPI gpi = (LPFN_GPI) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetProductInfo" );
        if ( gpi )
        {
            DWORD prodType = 0;
            BOOL ok = gpi( ver.dwMajorVersion, ver.dwMinorVersion, ver.wServicePackMajor, ver.wServicePackMinor, &prodType );
            if ( ok )
                printf( "  product type:                            %#x == %s\n", prodType, OSVersionString( prodType ) );
        }
    }
    else
    {
        DWORD dwVersion = GetVersion();
        DWORD major = (DWORD) ( LOBYTE( LOWORD( dwVersion ) ) );
        DWORD minor = (DWORD) ( HIBYTE( LOWORD( dwVersion ) ) );
        printf( "os version via GetVersion:\n" );
        printf( "  major.minor:                             %d.%d\n", major, minor );
    }
} //ShowOSVersion

BOOL WINAPI MonitorEnumProc( HMONITOR mon, HDC hdc, LPRECT rect, LPARAM param )
{
    static int monsofar = 0;

    if ( rect )
        printf( "  monitor %d rectangle:                     %d, %d, %d, %d\n",
                monsofar++, rect->left, rect->top, rect->right, rect->bottom );

    return TRUE;
} //MonitorEnumProc

void ShowMonitors()
{
    {
        HMODULE hmodUser32 = LoadLibraryA( "user32.dll" );
        if ( 0 == hmodUser32 )
        {
            printf( "\nCan't LoadLibrary user32.dll, error %d.\n", GetLastError() );
            return;
        }

        typedef HRESULT ( WINAPI *LPFN_EDM )( HDC, LPCRECT, MONITORENUMPROC, LPARAM );
        LPFN_EDM edm = (LPFN_EDM) GetProcAddress( hmodUser32, "EnumDisplayMonitors" );
        if ( edm )
        {
            printf( "monitor information via EnumDisplayMonitors:\n" );
            HRESULT hr = edm( 0, 0, MonitorEnumProc, 0 );
        }
        else
            printf( "Can't get proc address of EnumDisplayMonitors\n" );

        FreeLibrary( hmodUser32 );
    }

    {
        HMODULE hmodKernelBase = LoadLibraryA( "kernelbase.dll" );
        if ( 0 == hmodKernelBase )
        {
            printf( "Can't LoadLibrary kernelbase.dll, error %d.\n", GetLastError() );
            return;
        }

        typedef HRESULT ( WINAPI *LPFN_GIDS )( double * );
        LPFN_GIDS gids = (LPFN_GIDS) GetProcAddress( hmodKernelBase, "GetIntegratedDisplaySize" );
        if ( gids )
        {
            double inches = 0.0;
            HRESULT hr = gids( & inches );
            if ( S_OK == hr )
            {
                printf( "monitor information via GetIntegratedDisplaySize:\n" );
                printf( "  internal display size in inches:         %lf\n", inches );
            }
        }
        else
            printf( "Can't get proc address of GetIntegratedDisplaySize\n" );

        FreeLibrary( hmodKernelBase );
    }
} //ShowMonitors

#if defined( _M_IX86 ) || defined( _M_X64 )

#if _MSC_VER <= 1200

static void __cpuidex( int * result4, int codeeax, int codeecx )
{
    unsigned int a, b, c, d;
    __asm
    {
        mov eax, codeeax;
        mov ecx, codeecx
        mov ebx, 0
        mov edx, 0
        cpuid;
        mov a, eax;
        mov b, ebx;
        mov c, ecx;
        mov d, edx;
    }

    result4[0] = a;
    result4[1] = b;
    result4[2] = c;
    result4[3] = d;
} //__cpuidex

static void __cpuid( int * result4, int codeeax )
{
    __cpuidex( result4, codeeax, 0 );
} //__cpuid

#endif // _MSC_VER_1200

const char * edxFlags1[] =
{
    "fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
    "cx8", "apic", 0, "sep", "mtrr", "pge", "mca", "cmov",
    "pat", "pse-36", "psn", "clfsh", 0, "ds", "acpi", "mmx",
    "fxsr", "sse", "sse2", "ss", "htt", "tm", "ia64", "pbe"
};

const char * ecxFlags1[] =
{
    "sse3", "pclmulqdq", "dtes64", "monitor", "ds-cpl", "vmx", "smx", "est",
    "tm2", "ssse3", "cntx-id", "sdbg", "fma", "cx16", "xtpr", "pdcm",
    0, "pcid", "dca", "sse4.1", "sse4.2", "x2apic", "movbe", "popcnt",
    "tsc-deadline", "aes", "xsave", "osxsave", "avx", "f16c", "rdrnd", "hypervisor"
};

const char * ebxFlags70[] =
{
    "fsgsbase", "tsc_adj", "sgx", "bmi1", "hle", "avx2", "fdnex", "smep",
    "bmi2", "erms", "invpcid", "rtm", "rtm-m", 0, "mpx", "rdt-a",
    "avx512-f", "avx512-dq", "rdseed", "adx", "smap", "avx512-ifma", 0, "clflushopt",
    "clwb", "pt", "avx512-pf", "avx512-er", "avx512-cd", "sha", "avx512-bw", "avx512-vl"
};

const char * ecxFlags70[] =
{
    "prefetchwt1", "avx512-vbmi", "umip", "pku", "ospke", "waitpkg", "avx512-vbmi2", "cet-ss",
    "gfni", "vaes", "vpclmulqdq", "avx512-vnni", "avx512-bitalg", "tme", "avx512-vpopcntdq", 0,
    "la57", 0, 0, 0, 0, 0, "rdpid", "kl",
    "bus-lock_detect", "cidemote", 0, "movdiri", "movdir64b", "enqcmd", "sgx-lc", "pks"
};

const char * edxFlags70[] =
{
    0, 0, "avx512-4vnniw", "avx512-4fmaps", "fsrm", "uintr", 0, 0,
    "avx512-vp2intersect", "srdbs-ctrl", "mc-clear", "rtmaa", 0, "tsx_force_abort", "serialize", "hybrid",
    "tsxldtrk", 0, "pconfig", "lbr", "cet-ibt", 0, "amx-bf16", "avx512-fp16",
    "amx-tile", "amx-int8", "ibrs_ibpb", "stibp", "ld1_flush", "ia32_arch", "ia32_core", "ssbd"
};

void ShowFeatures( const char * banner, int i_bits, const char * flags[] )
{
    printf( banner );
    unsigned int bits = (unsigned int) i_bits;
    int bit = 1;
    for ( int i = 0; i < 32; i++ )
    {
        if ( 0 != i && ( 0 == ( i % 8 ) ) )
            printf( "\n                                           " );

        if ( 0 != flags[ i ] )
        {
            printf( flags[ i ] );

            printf( ( bits & bit ) ? "+" : "-" );
            if ( i != 31 )
                printf( ", " );
        }

        bit <<= 1;
    }

    printf( "\n" );
} //ShowFeatures

void ShowAssoc( unsigned int x )
{
    printf( "%#04x == ", x );
    if ( 1 == x )
        printf( "direct mapped" );
    else if ( 0xff == x )
        printf( "fully associative" );
    else
        printf( "n-way associative" );

    printf( "\n" );
} //ShowAssoc

const char * L23Assoc[] = {
    "disabled",
    "1 way (direct mapped)",
    "2 ways",
    "reserved",
    "4 ways",
    "reserved",
    "8 ways",
    "reserved"
    "16 ways",
    "reserved",
    "32 ways",
    "48 ways",
    "64 ways",
    "96 ways",
    "128 ways",
    "fully associative",
};

void ShowCPUID()
{
    printf( "cpu information from cpuid:\n" );
    int vals[ 4 ];  // eax, ebx, ecx, edx return values

    __cpuid( vals, 0x80000000 );
    int highestExtendedFunction = vals[ 0 ];

    __cpuid( vals, 0 );

    char cpumake[ 13 ];
    cpumake[ 12 ] = 0;
    * (unsigned int *) (&cpumake[0]) = vals[1];
    * (unsigned int *) (&cpumake[4]) = vals[3];
    * (unsigned int *) (&cpumake[8]) = vals[2];
    printf( "  cpu make:                                %s\n", cpumake );

    int highestFunction = vals[ 0 ];
    printf( "  highest cpuid / extended functions:      %#x / %#x\n", highestFunction, highestExtendedFunction );

    bool hyperThreadingAvailable = false;

    if ( highestFunction >= 1 )
    {
        __cpuid( vals, 1 );

        int steppingID = vals[ 0 ] & 0x7;
        printf( "  stepping id:                             %d\n", steppingID );

        int pType = ( vals[0] >> 12 ) & 3;
        printf( "  processor type:                          %s\n", 0 == pType ? "oem" :
                                                                   1 == pType ? "Intel Overdrive" :
                                                                   2 == pType ? "dual processor" :
                                                                   "reserved" );
        int extendedFamilyID = ( vals[ 0 ] >> 20 ) & 0x7f;
        int extendedModelID = ( vals[ 0 ] >> 16 ) & 0xf;
        int familyID = ( vals[ 0 ] >> 8 ) & 0xf;
        int model = ( vals[ 0 ] >> 4 ) & 0xf;
        int processorModel = model;
        if ( 6 == familyID || 15 == familyID )
            processorModel = ( extendedModelID << 4 ) + model;

        int family = familyID;
        if ( 15 == familyID )
            family = extendedFamilyID + familyID;

        printf( "  model / family:                          %d / %d\n", processorModel, family );

        int edx = vals[3];
        hyperThreadingAvailable = ( 0 != ( edx & 0x10000000 ) );
        printf( "  hyper-threading available:               %s\n", hyperThreadingAvailable ? "true" : "false" );

        if ( hyperThreadingAvailable )
        {
            // this is often inaccurate on older CPUs.

            int ebx = vals[ 1 ];
            int legacyLogicalProcessorCount = ( ( ebx >> 16 ) & 0xff );
            printf( "  legacy logical processor count:          %d\n", legacyLogicalProcessorCount );
        }

        int clflush = ( vals[1] >> 8 ) & 0xff;
        printf( "  cache line size:                         %d\n", clflush * 8 );

        assert( ( 32 * sizeof( char * ) ) == sizeof( edxFlags1 ) );
        assert( ( 32 * sizeof( char * ) ) == sizeof( ecxFlags1 ) );
        assert( ( 32 * sizeof( char * ) ) == sizeof( ebxFlags70 ) );
        assert( ( 32 * sizeof( char * ) ) == sizeof( ecxFlags70 ) );
        assert( ( 32 * sizeof( char * ) ) == sizeof( edxFlags70 ) );

        ShowFeatures( "  features edx:                            ", edx, edxFlags1 );
        int ecx = vals[2];
        ShowFeatures( "  features ecx:                            ", ecx, ecxFlags1 );
    }

    if ( highestFunction >= 4 && !strcmp( cpumake, "GenuineIntel" ) )
    {
        // note: this doesn't work on all Intel CPUs -- reporting is inconsistent.
        // But it seems to work on many current CPUs.

        __cpuid( vals, 4 );
        int cores = ( ( vals[0] >> 26 ) & 0x3f ) + 1;
        printf( "  Intel core count:                        %d\n", cores );
    }

    if ( highestExtendedFunction >= 0x80000004 )
    {
        int brand[13];
        brand[12] = 0;
        __cpuid( vals, 0x80000002 );
        brand[0] = vals[0];
        brand[1] = vals[1];
        brand[2] = vals[2];
        brand[3] = vals[3];
        __cpuid( vals, 0x80000003 );
        brand[4] = vals[0];
        brand[5] = vals[1];
        brand[6] = vals[2];
        brand[7] = vals[3];
        __cpuid( vals, 0x80000004 );
        brand[8] = vals[0];
        brand[9] = vals[1];
        brand[10] = vals[2];
        brand[11] = vals[3];

        // the brand can have leading spaces -- ignore them

        char * pBrand = (char *) brand;
        while ( ' ' == *pBrand )
            pBrand++;
        printf( "  brand:                                   %s\n", pBrand );
    }

    if ( highestFunction >= 7 )
    {
        __cpuidex( vals, 7, 0 );
        int ebx = vals[1];
        ShowFeatures( "  features 7/0 ebx:                        ", ebx, ebxFlags70 );
        int ecx = vals[2];
        ShowFeatures( "  features 7/0 ecx:                        ", ecx, ecxFlags70 );
        int edx = vals[3];
        ShowFeatures( "  features 7/0 edx:                        ", edx, edxFlags70 );
    }

    if ( g_fullInformation && highestExtendedFunction >= 0x80000005 && ( !strcmp( cpumake, "AuthenticAMD" ) ) )
    {
        __cpuid( vals, 0x80000005 );

        // eax 4MB entries

        unsigned int eax = vals[0];
        if ( 0 != eax )
        {
            printf( "  l1 2 and 4 meg cache and tlb identifiers:\n" );

            unsigned int assocL1 = ( eax >> 24 ) & 0xff;
            printf( "    data associativity:                    " );
            ShowAssoc( assocL1 );

            unsigned int twoMBTLB = ( eax >> 16 ) & 0xff;
            printf( "    data tlb entries for 2mb pages:        %d\n", twoMBTLB );

            assocL1 = ( eax >> 8 ) & 0xff;
            printf( "    code associativity:                    " );
            ShowAssoc( assocL1 );

            twoMBTLB = eax & 0xff;
            printf( "    code tlb entries for 2mb pages:        %d\n", twoMBTLB );
        }

        // ebx 4k entries

        unsigned int ebx = vals[1];
        if ( 0 != ebx )
        {
            printf( "  l1 4k page cache and tlb identifiers:\n" );

            unsigned int assocL1 = ( ebx >> 24 ) & 0xff;
            printf( "    data associativity:                    " );
            ShowAssoc( assocL1 );

            unsigned int fourkSize = ( ebx >> 16 ) & 0xff;
            printf( "    data 4k tlb entries:                   %d\n", fourkSize );

            assocL1 = ( ebx >> 8 ) & 0xff;
            printf( "    code associativity:                    " );
            ShowAssoc( assocL1 );

            fourkSize = ebx & 0xff;
            printf( "    code 4k tlb entries:                   %d\n", fourkSize );
        }

        // ecx data cache and tlb identifies

        unsigned int ecx = vals[2];
        if ( 0 != ecx )
        {
            printf( "  l1 data cache and identifiers:\n" );

            unsigned int sizeK = ( ecx >> 24 ) & 0xff;
            printf( "    data size in kb:                       %d\n", sizeK );

            unsigned int assoc = ( ecx >> 16 ) & 0xff;
            printf( "    data associativity:                    " );
            ShowAssoc( assoc );

            unsigned int cachelines = ( ecx >> 8 ) & 0xff;
            printf( "    cache lines per tag:                   %d\n", cachelines );

            unsigned cachelinesize = ecx & 0xff;
            printf( "    cache line size:                       %d\n", cachelinesize );
        }

        // edx code cache and tlb identifies

        unsigned int edx = vals[3];
        if ( 0 != edx )
        {
            printf( "  l1 code cache and identifiers:\n" );

            unsigned int sizeK = ( edx >> 24 ) & 0xff;
            printf( "    code size in kb:                       %d\n", sizeK );

            unsigned int assoc = ( edx >> 16 ) & 0xff;
            printf( "    code associativity:                    " );
            ShowAssoc( assoc );

            unsigned int cachelines = ( edx >> 8 ) & 0xff;
            printf( "    cache lines per tag:                   %d\n", cachelines );

            unsigned cachelinesize = edx & 0xff;
            printf( "    cache line size:                       %d\n", cachelinesize );
        }
    }

    if ( g_fullInformation && highestExtendedFunction >= 0x80000006 )
    {
        __cpuid( vals, 0x80000006 );

        unsigned int eax = vals[ 0 ];
        if ( 0 != eax && ( !strcmp( cpumake, "AuthenticAMD" ) ) )
        {
            printf( "  l2 cache 2 and 4 mb:\n" );

            unsigned int l2_2and4megDataAssoc = ( eax >> 28 ) & 0xf;
            unsigned int l2_2and4megDataTLB = ( eax >> 16 ) & 0xfff;
            printf( "    data associativity:                    %d == %s\n", l2_2and4megDataAssoc, L23Assoc[ l2_2and4megDataAssoc ] );
            printf( "    data tlb entries for 2mb pages:        %d\n", l2_2and4megDataTLB );

            unsigned int l2_2and4megCodeAssoc = ( eax >> 12 ) & 0xf;
            unsigned int l2_2and4megCodeTLB = ( eax ) & 0xfff;
            printf( "    code associativity:                    %d == %s\n", l2_2and4megCodeAssoc, L23Assoc[ l2_2and4megCodeAssoc ] );
            printf( "    code tlb entries for 2mb pages:        %d\n", l2_2and4megCodeTLB );
        }

        unsigned int ebx = vals[ 1 ];
        if ( 0 != ebx && ( !strcmp( cpumake, "AuthenticAMD" ) ) )
        {
            printf( "  l2 cache for 4kb pages:\n" );

            unsigned int l2_4kDataAssoc = ( ebx >> 28 ) & 0xf;
            unsigned int l2_4kDataTLB = ( ebx >> 16 ) & 0xfff;
            printf( "    data associativity:                    %d == %s\n", l2_4kDataAssoc, L23Assoc[ l2_4kDataAssoc ] );
            printf( "    data tlb entries for 4k pages:         %d\n", l2_4kDataTLB );

            unsigned int l2_4kCodeAssoc = ( ebx >> 12 ) & 0xf;
            unsigned int l2_4kCodeTLB = ( ebx ) & 0xfff;
            printf( "    code associativity:                    %d == %s\n", l2_4kCodeAssoc, L23Assoc[ l2_4kCodeAssoc ] );
            printf( "    code tlb entries for 4k pages:         %d\n", l2_4kCodeTLB );
        }

        unsigned int ecx = vals[ 2 ];
        if ( 0 != ecx ) // Intel and AMD
        {
            printf( "  l2 cache in kb:\n" );

            unsigned int l2_sizekb = ( ecx >> 16 ) & 0xffff;
            printf( "    size in kb:                            %d\n", l2_sizekb );

            unsigned int l2_assoc = ( ecx >> 12 ) & 0xf;
            printf( "    associativity:                         %s\n", L23Assoc[ l2_assoc ] );

            unsigned int l2_linesPerTag = ( ecx >> 8 ) & 0xf;
            printf( "    lines per tag:                         %d\n", l2_linesPerTag );

            unsigned int l2_linesize = ( ecx & 0xff );
            printf( "    line size in bytes:                    %d\n", l2_linesize );
        }

        unsigned int edx = vals[ 3 ];
        if ( 0 != edx && ( !strcmp( cpumake, "AuthenticAMD" ) ) )
        {
            printf( "  l3 cache:\n" );

            // I'm getting an inaccurate number for l3_size
            unsigned int l3_size = ( edx >> 18 ) & 0x3fff; // 14 bits
            printf( "    size in range:                         %d <= X < %d\n", 512 * l3_size, 512 * ( 1 + l3_size ) );

            unsigned int l3_assoc = ( edx >> 12 ) & 0xf;
            printf( "    associativity:                         %s\n", L23Assoc[ l3_assoc ] );

            unsigned int l3_linesPerTag = ( edx >> 8 ) & 0xf;
            printf( "    lines per tag:                         %d\n", l3_linesPerTag );

            unsigned int l3_linesize = ( edx & 0xff );
            printf( "    line size in bytes:                    %d\n", l3_linesize );
        }
    }

    if ( highestExtendedFunction >= 0x80000008 )
    {
        __cpuid( vals, 0x80000008 );

        int ecx = vals[ 2 ];
        if ( !strcmp( cpumake, "AuthenticAMD" ) ) // Intel CPUs have 0s; it's AMD-only
        {
            int physicalCoreCount = 1 + ( ecx & 0xff );
            printf( "  AMD physical / logical cores:            %d / %d\n", physicalCoreCount,
                    hyperThreadingAvailable ? 2 * physicalCoreCount : physicalCoreCount );

            int log2MaxApicId = ( ( ecx >> 12 ) & 0xf );
            printf( "  log2 of max apic id:                     %d\n", log2MaxApicId );
        }

        int eax = vals[ 0 ];
        int physicalAddressBits = ( eax & 0xff );
        int linearAddressBits = ( ( eax >> 8 ) & 0xff );
        printf( "  physical / linear address bits:          %d / %d\n", physicalAddressBits, linearAddressBits );
    }
} //ShowCPUID

void showchar( char ch )
{
    if ( ( 0xff == ch ) || ( ( ch >= 0 ) && ( ch <= 0x1f ) ) )
        ch = '.';
    printf( "%c", ch );
} //showchar

void showvals( int fun, int * vals )
{
    printf( "  %#010x: %#010x %#010x %#010x %#010x        ", fun, vals[0], vals[1], vals[2], vals[3] );

    for ( int v = 0; v < 4; v++ )
    {
        unsigned x = vals[ v ];
        showchar( x & 0xff );
        showchar( ( x >> 8 ) & 0xff );
        showchar( ( x >> 16 ) & 0xff );
        showchar( ( x >> 24 ) & 0xff );

        printf( " " );
    }

    printf( "\n" );
} //showvals

void ShowAllCPUID()
{
    printf( "all cpu information from cpuid, function: eax ebx ecx edx:\n" );
    int vals[ 4 ];  // eax, ebx, ecx, edx return values

    __cpuid( vals, 0 );
    int highestFunction = vals[ 0 ];

    for ( int i = 0; i <= highestFunction; i++ )
    {
        __cpuid( vals, i );
        showvals( i, vals );
    }

    __cpuid( vals, 0x80000000 );
    unsigned int highestExtendedFunction = vals[ 0 ];

    // older CPUs return random values, so do some basic validation

    if ( ( highestExtendedFunction >= 0x80000000 ) && ( ( highestExtendedFunction - 0x80000000 ) < 0x100 ) )
    {
        for ( int i = 0x80000000; i <= highestExtendedFunction; i++ )
        {
            __cpuid( vals, i );
            showvals( i, vals );
        }
    }
} //ShowAllCPUID

#endif // defined( _M_IX86 ) || defined( _M_X64 )

const char * ArchitectureString( USHORT m )
{
    if ( 0x014c == m )
        return "Intel 386";
    if ( 0x8664 == m )
        return "AMD 64";
    if ( 0xAA64 == m )
        return "Arm 64";
    if ( 0 == m )
        return "native";
    return "(not recognized)";
} //ArchectureString

void ShowArchitectureInfo()
{
    USHORT processMachine = 0, nativeMachine = 0;
    typedef BOOL ( WINAPI *LPFN_IW64P2 )( HANDLE, USHORT *, USHORT * );
    LPFN_IW64P2 iw64p2 = (LPFN_IW64P2) GetProcAddress( GetModuleHandleA( "kernel32" ), "IsWow64Process2" );
    BOOL ok_iw64p2 = FALSE;
    if ( 0 != iw64p2 )
        ok_iw64p2 = iw64p2( GetCurrentProcess(), &processMachine, &nativeMachine );

    typedef BOOL ( WINAPI *LPFN_GPI )( HANDLE, PROCESS_INFORMATION_CLASS_FAKE, LPVOID, DWORD );
    LPFN_GPI gpi = (LPFN_GPI) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetProcessInformation" );
    if ( 0 != gpi )
    {
        PROCESS_MACHINE_INFORMATION_FAKE info = { 0 };
        BOOL ok_gpi = gpi( GetCurrentProcess(), ProcessMachineTypeInfoFake, &info, sizeof( info ) );
        if ( ok_gpi )
            processMachine = info.ProcessMachine; // necessary because iw64p2 returns 0, not something interesting

    }

    if ( ok_iw64p2 )
    {
        printf( "process and system architecture via IsWow64Process2 and GetProcessInformation:\n" );
        printf( "  process / system architecture:           %#x == %s / %#x == %s\n",
                processMachine, ArchitectureString( processMachine ),
                nativeMachine, ArchitectureString( nativeMachine ) );
    }
} //ShowArchitectureInfo

typedef LSTATUS ( WINAPI *LPFN_RQVE )( HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD );

void ShowRegString( LPFN_RQVE rqve, HKEY hKey, const char * name )
{
    char buf[ 100 ] = {0};
    DWORD keyType, keySize = _countof( buf );
    int len = strlen( name );
    if ( ( !rqve( hKey, name, 0, &keyType, (PBYTE) buf, &keySize ) ) && ( REG_SZ == keyType ) )
        printf( "  %s: %-*s  %s\n", name, 37 - len, " ", buf );
} //ShowRegString

void ShowRegCentralProcessor()
{
    HMODULE hmodAdvapi32 = LoadLibraryA( "advapi32.dll" );
    if ( hmodAdvapi32 )
    {
        typedef LSTATUS ( WINAPI *LPFN_ROKE )( HKEY, LPCSTR, DWORD, REGSAM, PHKEY );
        LPFN_ROKE roke = (LPFN_ROKE) GetProcAddress( hmodAdvapi32, "RegOpenKeyExA" );

        LPFN_RQVE rqve = (LPFN_RQVE) GetProcAddress( hmodAdvapi32, "RegQueryValueExA" );

        typedef LSTATUS ( WINAPI *LPFN_RCK )( HKEY );
        LPFN_RCK rck = (LPFN_RCK) GetProcAddress( hmodAdvapi32, "RegCloseKey" );

        if ( roke && rqve && rck )
        {
            const char *csName = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
            HKEY hKey;
            if ( ERROR_SUCCESS == roke( HKEY_LOCAL_MACHINE, csName, 0, KEY_READ, &hKey ) )
            {
                printf( "processor info via registry key HKLM\\%s:\n", csName );

                ShowRegString( rqve, hKey, "Identifier" );
                ShowRegString( rqve, hKey, "ProcessorNameString" );
                ShowRegString( rqve, hKey, "VendorIdentifier" );

                rck( hKey );
            }
        }

        FreeLibrary( hmodAdvapi32 );
    }
} //ShowRegCentralProcessor

void ShowRegBiosInfo()
{
    HMODULE hmodAdvapi32 = LoadLibraryA( "advapi32.dll" );
    if ( hmodAdvapi32 )
    {
        typedef LSTATUS ( WINAPI *LPFN_ROKE )( HKEY, LPCSTR, DWORD, REGSAM, PHKEY );
        LPFN_ROKE roke = (LPFN_ROKE) GetProcAddress( hmodAdvapi32, "RegOpenKeyExA" );

        LPFN_RQVE rqve = (LPFN_RQVE) GetProcAddress( hmodAdvapi32, "RegQueryValueExA" );

        typedef LSTATUS ( WINAPI *LPFN_RCK )( HKEY );
        LPFN_RCK rck = (LPFN_RCK) GetProcAddress( hmodAdvapi32, "RegCloseKey" );

        if ( roke && rqve && rck )
        {
            const char *csName = "HARDWARE\\DESCRIPTION\\System\\BIOS";
            HKEY hKey;
            if ( ERROR_SUCCESS == roke( HKEY_LOCAL_MACHINE, csName, 0, KEY_READ, &hKey ) )
            {
                printf( "bios info via registry key HKLM\\%s:\n", csName );

                ShowRegString( rqve, hKey, "BaseBoardManufacturer" );
                ShowRegString( rqve, hKey, "BaseBoardProduct" );
                ShowRegString( rqve, hKey, "BaseBoardVersion" );
                ShowRegString( rqve, hKey, "BiosVendor" );
                ShowRegString( rqve, hKey, "BiosReleaseDate" );
                ShowRegString( rqve, hKey, "BiosVersion" );
                ShowRegString( rqve, hKey, "SystemFamily" );
                ShowRegString( rqve, hKey, "SystemManufacturer" );
                ShowRegString( rqve, hKey, "SystemProductName" );
                ShowRegString( rqve, hKey, "SystemDKU" );
                ShowRegString( rqve, hKey, "SystemVersion" );

                rck( hKey );
            }
        }

        FreeLibrary( hmodAdvapi32 );
    }
} //ShowRegBiosInfo

void ShowGraphicsMemorySize( const char * pmsg, SIZE_T b )
{
    SIZE_T mb = b / ( 1024 * 1024 );

#ifdef _WIN64 // VS6 doesn't understand %z, so this has to be hard-coded
    if ( 0 == mb )
        printf( "    %-38s %lld bytes\n", pmsg, b );
    else
        printf( "    %-38s %lld megabytes\n", pmsg, mb );
#else
    if ( 0 == mb )
        printf( "    %-38s %ld bytes\n", pmsg, b );
    else
        printf( "    %-38s %ld megabytes\n", pmsg, mb );
#endif
} //ShowGraphicsMemorySize

void ShowGraphicsAdapters()
{
    HMODULE hmodDXGI  = LoadLibraryA( "dxgi.dll" );
    if ( hmodDXGI )
    {
        typedef HRESULT ( WINAPI *LPFN_CDXGIF ) ( REFIID, void ** );
        LPFN_CDXGIF cdxgif = (LPFN_CDXGIF) GetProcAddress( hmodDXGI, "CreateDXGIFactory" );
        if ( cdxgif )
        {
            IDXGIFactory * pFactory = 0;
            // HRESULT hr = cdxgif( __uuidof( IDXGIFactory ), (void**) &pFactory );
            HRESULT hr = cdxgif( GUID_IDXGIFactory, (void**) &pFactory );
            if ( FAILED( hr ) )
                printf( "unable to create DXGIFactory, error %#x\n", hr );
            else
            {
                printf( "display adapters:\n" );
                UINT i = 0;
                do
                {
                    IDXGIAdapter * pAdapter = 0;
                    hr = pFactory->EnumAdapters( i, &pAdapter );

                    if ( hr == DXGI_ERROR_NOT_FOUND )
                        break;

                    printf( "  display adapter %u:\n", i );

                    DXGI_ADAPTER_DESC adapterDesc; // modern memory sizes don't fit in 4 bytes for 32-bit versions of SI.
                    hr = pAdapter->GetDesc( &adapterDesc );
                    if ( SUCCEEDED( hr ) )
                    {
                        printf( "    %-38s %ws\n", "adapter description:", adapterDesc.Description );
                        printf( "    vendor, device, and subystem PCI IDs:  %#x, %#x, %#x\n", adapterDesc.VendorId, adapterDesc.DeviceId, adapterDesc.SubSysId );
                        ShowGraphicsMemorySize( "dedicated video memory:", adapterDesc.DedicatedVideoMemory );
                        ShowGraphicsMemorySize( "dedicated system memory:", adapterDesc.DedicatedSystemMemory );
                        ShowGraphicsMemorySize( "shared system memory:", adapterDesc.SharedSystemMemory );
                    }
                    else
                        printf( "    can't get display adapter desc, error %#x\n", hr );

                    i++;
                } while( true );
            }
        }
        FreeLibrary( hmodDXGI );
    }
    else
        printf( "can't loadlibrary dxgi.dll\n" );
} //ShowGraphicsAdapters

void usage()
{
    printf( "si [-c] [-f]\n" );
    printf( "System Information\n" );
    printf( "    arguments:    [-c]       Show all paramaterless cpuid information unparsed (x86/x64 only)\n" );
    printf( "                  [-f]       Show full information -- this is more verbose\n" );
    exit( 0 );
} //usage

int main( int argc, char * argv[] )
{
    // set this early to get physical numbers for monitor rectangles

    HMODULE hmodUser32 = LoadLibraryA( "user32.dll" );
    if ( hmodUser32 )
    {
        typedef BOOL ( WINAPI *LPFN_SPDAC )( DPI_AWARENESS_CONTEXT );
        LPFN_SPDAC spdac = (LPFN_SPDAC) GetProcAddress( hmodUser32, "SetProcessDpiAwarenessContext" );
        if ( spdac )
            spdac( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

        FreeLibrary( hmodUser32 );
    }

    bool allCpuid = false;

    for ( int a = 1; a < argc; a++ )
    {
        const char * parg = argv[ a ];

        if ( '/' == *parg || '-' == *parg )
        {
            char c = tolower( parg[1] );

            if ( 'c' == c )
                allCpuid = true;
            else if ( 'f' == c )
                g_fullInformation = true;
            else
                usage();
        }
        else
            usage();
    }

#if defined( _M_IX86 ) || defined( _M_X64 )
    if ( allCpuid )
        ShowAllCPUID();
#endif

    ShowNames();

    bool ok = AttemptNewAPIForProcessorInfo();
    if ( !ok )
        ok = AttemptOldAPIForProcessorInfo();

#if defined( _M_IX86 ) || defined( _M_X64 )
    ShowCPUID();
#endif

    ShowRegCentralProcessor();
    ShowRegBiosInfo();
    ShowSystemMemory();
    ShowCPUSpeed();
    ShowSystemInfo();
    ShowArchitectureInfo();
    ShowHypervisorInfo();
    ShowNetworkAdapters();
    ShowGraphicsAdapters();
    ShowMonitors();
    ShowOSVersion();
    ShowDrives();

    return 0;
} //main


