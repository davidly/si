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
// Todo: - handle CPUs with both efficiency and performance cores
//       - ARM64 equivalent of CPUID
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
#include <winioctl.h>
#include <iphlpapi.h>

// Microsoft accidentally omitted this from headers decades ago...

#ifndef PROCESSOR_POWER_INFORMATION

    typedef struct _PROCESSOR_POWER_INFORMATION {
      ULONG Number;
      ULONG MaxMhz;
      ULONG CurrentMhz;
      ULONG MhzLimit;
      ULONG MaxIdleState;
      ULONG CurrentIdleState;
    } PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

#endif

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

// Enable building with VS6 tools so the binary can run in Win98.
// That means replicating all of these definitions.

#if _MSC_VER == 1200

    #ifndef LTP_PC_SMT
        #define LTP_PC_SMT 0x1
    #endif

    #define _countof( x ) ( sizeof( x ) / sizeof( x[0] ) )

    typedef unsigned int ULONG_PTR;
    typedef HRESULT NTSTATUS;
    
    #define PROCESSOR_ARCHITECTURE_AMD64 9

    #ifndef PROCESSOR_ARCHITECTURE_ARM64
        #define PROCESSOR_ARCHITECTURE_ARM64 12
    #endif

    typedef HANDLE HMONITOR;
    typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

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

#endif // _MSC_VER_1200

void PrintNumberWithCommas( __int64 n )
{
    if ( n < 0 )
    {
        printf( "-" );
        PrintNumberWithCommas( -n );
        return;
    }
   
    if ( n < 1000 )
    {
        printf( "%lld", n );
        return;
    }

    PrintNumberWithCommas( n / 1000 );
    printf( ",%03lld", n % 1000 );
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

bool AttemptNewAPIForProcessorInfo()
{
    DWORD returnLength = 0;

    typedef BOOL ( WINAPI *LPFN_GLPIEX )( LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD );
    LPFN_GLPIEX glpi = (LPFN_GLPIEX) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetLogicalProcessorInformationEx" );
    if ( NULL == glpi ) 
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

    bool shownL1 = false, shownL2 = false, shownL3 = false;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD logicalProcessorCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
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
                processorCoreCount++;
    
                // A hyperthreaded core supplies more than one logical processor.

                logicalProcessorCount += ( LTP_PC_SMT == ptr->Processor.Flags ) ? 2 : 1;
                break;
            }
            case RelationCache:
            {
                // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.
    
                CACHE_RELATIONSHIP * Cache = & ( ptr->Cache );
    
                if ( 1 == Cache->Level )
                {
                    if ( !shownL1 )
                    {
                        printf( "  L1 cache / line size:                    %dk / %d\n", Cache->CacheSize / 1024, Cache->LineSize );
                        shownL1 = true;
                    }
                    processorL1CacheCount++;
                }
                else if ( 2 == Cache->Level )
                {
                    if ( !shownL2 )
                    {
                        printf( "  L2 cache / line size:                    %dk / %d\n", Cache->CacheSize / 1024, Cache->LineSize );
                        shownL2 = true;
                    }
                    processorL2CacheCount++;
                }
                else if ( 3 == Cache->Level )
                {
                    if ( !shownL3 )
                    {
                        printf( "  L3 cache / line size:                    %dk / %d\n", Cache->CacheSize / 1024, Cache->LineSize );
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

    printf( "  number of processor L1 / L2 / L3 caches: %d / %d / %d\n", processorL1CacheCount,processorL2CacheCount, processorL3CacheCount );
    printf( "  number of NUMA nodes:                    %d\n", numaNodeCount );
    printf( "  number of physical processor packages:   %d\n", processorPackageCount);
    printf( "  number of processor cores:               %d\n", processorCoreCount );
    printf( "  number of logical processors:            %d\n", logicalProcessorCount );
    
    free( buffer );

    return true;
} //AttemptNewAPIForProcessorInfo

bool AttemptOldAPIForProcessorInfo()
{
    DWORD returnLength = 0;
    typedef BOOL ( WINAPI *LPFN_GLPI )( PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD );
    LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetLogicalProcessorInformation" );
    if (NULL == glpi) 
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
                        printf( "L1 cache / line size:                    %dk / %d\n", Cache->Size / 1024, Cache->LineSize );
                        shownL1 = true;
                    }
                    processorL1CacheCount++;
                }
                else if ( 2 == Cache->Level )
                {
                    if ( !shownL2 )
                    {
                        printf( "L2 cache / line size:                    %dk / %d\n", Cache->Size / 1024, Cache->LineSize );
                        shownL2 = true;
                    }
                    processorL2CacheCount++;
                }
                else if ( 3 == Cache->Level )
                {
                    if ( !shownL3 )
                    {
                        printf( "L3 cache / line size:                    %dk / %d\n", Cache->Size / 1024, Cache->LineSize );
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
    printf( "  number of processor L1 / L2 / L3 caches: %d / %d / %d\n", processorL1CacheCount,processorL2CacheCount, processorL3CacheCount );
    printf( "  number of NUMA nodes:                    %d\n", numaNodeCount );
    printf( "  number of physical processor packages:   %d\n", processorPackageCount);
    printf( "  number of processor cores:               %d\n", processorCoreCount );
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
            ULONGLONG meminGig = meminMeg / oneK;

            // vs6 printf can't print two 64 bit integers in one format string
            printf( "  total usable physical memory:            %lld MB == ", meminMeg );
            printf( "%lld GB\n", meminGig );
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

        if ( DeviceIoControl( hDevice, IOCTL_DISK_GET_PARTITION_INFO, 0, 0, &pi, sizeof( pi ), &dwRet, 0 ) )
        {
            _int64 low = pi.PartitionLength.LowPart;
            _int64 hi = pi.PartitionLength.HighPart;
            low += (hi << 32);
            low /= (1024 * 1024);
            printf(" %11ldm             ",(DWORD) low);
            fOk = TRUE;
        }

        CloseHandle( hDevice );
    }

    if ( !fOk )
        printf("                          ");
} //PartitionSize

VOID VolumeInfo( char * pcDriveName )
{
    char acVolume[50], acFS[50];
  
    if ( GetVolumeInformationA( pcDriveName, acVolume, sizeof( acVolume ), 0, 0, 0, acFS, sizeof( acFS ) ) )
        printf( "  %5s %24s", acFS, acVolume );
    else
        printf("      ?                         ");
  
    DWORD dwSPerC,dwBPerS,dwFreeClusters,dwClusters;
    _int64 onemeg = (_int64) 1024 * (_int64) 1024;

    typedef BOOL ( WINAPI *LPFN_GDFSEX ) ( const char *, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER );
    LPFN_GDFSEX gdfsex = (LPFN_GDFSEX) GetProcAddress( GetModuleHandleA( "kernel32" ), "GetDiskFreeSpaceExA" );
    if ( 0 != gdfsex )
    {
        ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
        BOOL ok = gdfsex( pcDriveName, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes );
        if ( ok )
        {
            // vs6 printf can't print two 64 bit integers in one format string
            printf( " %11lldm", totalNumberOfBytes.QuadPart / onemeg );
            printf( " %11lldm", freeBytesAvailable.QuadPart / onemeg );
        }
    }
    else if ( GetDiskFreeSpaceA( pcDriveName, &dwSPerC, &dwBPerS, &dwFreeClusters, &dwClusters ) )
    {
        _int64 spc = dwSPerC;
        _int64 sbps = dwBPerS;
        _int64 fc = dwFreeClusters;
        _int64 c = dwClusters;

        printf( " %11ldm %11ldm", (DWORD) ( (spc * sbps * c) / onemeg ), (DWORD) ( (spc * sbps * fc) / onemeg ) );
    }
    else
    {
        PartitionSize( pcDriveName[0] );
    }
} //VolumeInfo

void ShowDrives()
{
    printf( "drive info via GetDriveType, GetDiskFreeSpace(Ex):\n" );
    printf("      type      fs                   volume        total         free\n");

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

                    SetErrorMode(1); // no popups if no disk in drive!

                    if ( acRootPath[0] != 'a' && acRootPath[0] != 'b' )
                        VolumeInfo( acRootPath );
                    else
                        printf("      -");

                    SetErrorMode(0);
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

bool ShowCPUSpeed()
{
    typedef HRESULT ( WINAPI *LPFN_CNTPI )( POWER_INFORMATION_LEVEL, PVOID, ULONG, PVOID, ULONG );
    HMODULE hmod = LoadLibraryA( "powrprof.dll" );
    if ( NULL == hmod )
    {
        printf( "\nCan't LoadLibrary powrprof.dll, error %d.\n", GetLastError() );
        return false;
    }

    LPFN_CNTPI cntpi = (LPFN_CNTPI) GetProcAddress( hmod, "CallNtPowerInformation" );
    if ( NULL == cntpi ) 
    {
        printf( "\nCallNtPowerInformation is not supported, error %d.\n", GetLastError() );
        FreeLibrary( hmod );
        return false;
    }

    SYSTEM_INFO si;
    GetSystemInfo( &si );
    DWORD bufferSize = si.dwNumberOfProcessors * sizeof( PROCESSOR_POWER_INFORMATION );
    PROCESSOR_POWER_INFORMATION * ppi = (PROCESSOR_POWER_INFORMATION *) malloc( bufferSize );
    HRESULT hr = cntpi( ProcessorInformation, 0, 0, ppi, bufferSize );
    if ( S_OK == hr )
    {
        printf( "cpu speed via CallNtPowerInformation:\n" );

        // Note: on the AMD 5950x all three of the values are identical. On Intel CPUs these work.

        for ( DWORD i = 0; i < 1 && i < si.dwNumberOfProcessors; i++ )
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
            printf( "    ip address / subnet mask / gateway:    %s / %s / %s\n", pAdapter->IpAddressList.IpAddress.String,
                                                                                 pAdapter->IpAddressList.IpMask.String,
                                                                                 pAdapter->GatewayList.IpAddress.String );
            printf( "    dhcp enabled:                          %s\n", pAdapter->DhcpEnabled ? "true" : "false" );
                                                              
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
    if ( NULL == hmodAdvApi )
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
    HMODULE hmod = LoadLibraryA( "user32.dll" );
    if ( NULL == hmod )
    {
        printf( "\nCan't LoadLibrary user32.dll, error %d.\n", GetLastError() );
        return;
    }

    typedef HRESULT ( WINAPI *LPFN_EDM )( HDC, LPCRECT, MONITORENUMPROC, LPARAM );
    LPFN_EDM edm = (LPFN_EDM) GetProcAddress( hmod, "EnumDisplayMonitors" );
    if ( edm )
    {
        printf( "monitor information via EnumDisplayMonitors:\n" );
        HRESULT hr = edm( 0, 0, MonitorEnumProc, 0 );
    }
    else
        printf( "Can't get proc address of EnumDisplayMonitors\n" );

    FreeLibrary( hmod );

    HMODULE hmodKernelBase = LoadLibraryA( "kernelbase.dll" );
    if ( NULL == hmodKernelBase )
    {
        printf( "\nCan't LoadLibrary kernelbase.dll, error %d.\n", GetLastError() );
        return;
    }

    typedef HRESULT ( WINAPI *LPFN_GIDS )( double *);
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
        printf( "Can't get proc address of GetIntegratedDisplaySize" );

    FreeLibrary( hmodKernelBase );
} //ShowMonitors

#if defined( _M_IX86 ) || defined( _M_X64 )

#if _MSC_VER <= 1200

static void __cpuid( int * result4, int codeeax )
{
    unsigned int a, b, c, d;
    __asm
    {
        mov eax, codeeax;
        mov ecx, 0
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
} //__cpuid

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
        __cpuid( vals, 0x80000000 );
        if ( vals[ 0 ] > 0x80000004 )
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
            char acBrand[ 49 ];
            strcpy( acBrand, pBrand );
            printf( "  brand:                                   %s\n", acBrand );
        }
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

#endif // defined( _M_IX86 ) || defined( _M_X64 )

extern "C" int __cdecl wmain( int argc, WCHAR * argv[] )
{
    ShowNames();

    bool ok = AttemptNewAPIForProcessorInfo();
    if ( !ok )
        ok = AttemptOldAPIForProcessorInfo();

    ShowCPUID();
    ShowSystemMemory();
    ShowCPUSpeed();
    ShowSystemInfo();
    ShowNetworkAdapters();
    ShowOSVersion();
    ShowMonitors();
    ShowDrives();

    return 0;
} //wmain
