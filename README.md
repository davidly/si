# si
System Information. Command-line Windows app that shows info about the hardware and OS.

SI attempts to use the latest APIs to retrieve information, but will fall back to older APIs
going back to Windows 98. 

Instructions for building using different tool sets are in the source file. I build for Windows 98
using old tools/headers so the binary can run on all subsequent versions of Windows . It'll
also build with the latest tools, but the resulting binary can't even run on Windows XP.

Sample output:

    all cpu information from cpuid, function: eax ebx ecx edx:
      0000000000: 0x00000020 0x756e6547 0x6c65746e 0x49656e69         ... Genu ntel ineI 
      0x00000001: 0x000906a3 0x11800800 0xfffaf38b 0xbfcbfbff        £... ..€. ‹óúÿ ÿûË¿ 
      0x00000002: 0x00feff01 0x000000f0 0000000000 0000000000        .ÿþ. ð... .... .... 
      0x00000003: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000004: 0xfc004121 0x02c0003f 0x0000003f 0000000000        !A.ü ?.À. ?... .... 
      0x00000005: 0x00000040 0x00000040 0x00000003 0x00002020        @... @... ....   .. 
      0x00000006: 0x009f8ff3 0x00000002 0x00000409 0x00020003        óŸ. .... .... .... 
      0x00000007: 0x00000001 0x239c27a9 0x184027a4 0xbc18c410        .... ©'œ# ¤'@. .Ä.¼ 
      0x00000008: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000009: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x0000000a: 0x07300605 0000000000 0x00000007 0x00008603        ..0. .... .... .†.. 
      0x0000000b: 0x00000001 0x00000002 0x00000100 0x00000011        .... .... .... .... 
      0x0000000c: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x0000000d: 0x00000007 0x00000340 0x00000340 0000000000        .... @... @... .... 
      0x0000000e: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x0000000f: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000010: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000011: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000012: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000013: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000014: 0x00000001 0x0000005f 0x00000007 0000000000        .... _... .... .... 
      0x00000015: 0x00000002 0x00000068 0x0249f000 0000000000        .... h... .ðI. .... 
      0x00000016: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000017: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000018: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x00000019: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x0000001a: 0x40000001 0000000000 0000000000 0000000000        ...@ .... .... .... 
      0x0000001b: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x0000001c: 0x4000000b 0x00000007 0x00000007 0000000000        ...@ .... .... .... 
      0x0000001d: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x0000001e: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x0000001f: 0x00000001 0x00000002 0x00000100 0x00000011        .... .... .... .... 
      0x00000020: 0000000000 0x00000001 0000000000 0000000000        .... .... .... .... 
      0x80000000: 0x80000008 0000000000 0000000000 0000000000        ...€ .... .... .... 
      0x80000001: 0000000000 0000000000 0x00000121 0x2c100800        .... .... !... ..., 
      0x80000002: 0x68743231 0x6e654720 0x746e4920 0x52286c65        12th  Gen  Int el(R 
      0x80000003: 0x6f432029 0x54286572 0x6920294d 0x32312d37        ) Co re(T M) i 7-12 
      0x80000004: 0x00503038 0000000000 0000000000 0000000000        80P. .... .... .... 
      0x80000005: 0000000000 0000000000 0000000000 0000000000        .... .... .... .... 
      0x80000006: 0000000000 0000000000 0x05007040 0000000000        .... .... @p.. .... 
      0x80000007: 0000000000 0000000000 0000000000 0x00000100        .... .... .... .... 
      0x80000008: 0x00003027 0000000000 0000000000 0000000000        '0.. .... .... .... 
    computer name:                             DAVID-NANO
    user name:                                 david
    processor info via GetLogicalProcessorInformationEx:
      cache count / size / line size:
        L1 data:                               6 / 48K / 64
        L1 instruction:                        6 / 32K / 64
        L2 unified:                            6 / 1280K / 64
        L3 unified:                            1 / 24576K / 64
        L1 data:                               8 / 32K / 64
        L1 instruction:                        8 / 64K / 64
        L2 unified:                            2 / 2048K / 64
      core count / efficiency / hyperthreads:  6 / 1 / yes
      core count / efficiency / hyperthreads:  8 / 0 / no
      number of NUMA nodes:                    1
      number of physical processor packages:   1
      number of cores / processor units:       14
      number of logical processors:            20
      core hex masks at efficiency 0 (slower): 1000 2000 4000 8000 10000 20000 40000 80000
      core hex masks at efficiency 1 (faster): 3 c 30 c0 300 c00
    cpu information from cpuid:
      cpu make:                                GenuineIntel
      highest cpuid / extended functions:      0x20 / 0x80000008
      stepping id:                             3
      processor type:                          oem
      model / family:                          154 / 6
      hyper-threading available:               true
      legacy logical processor count:          128
      cache line size:                         64
      features edx:                            fpu+, vme+, de+, pse+, tsc+, msr+, pae+, mce+, 
                                               cx8+, apic+, sep+, mtrr+, pge+, mca+, cmov+, 
                                               pat+, pse-36+, psn-, clfsh+, ds-, acpi+, mmx+, 
                                               fxsr+, sse+, sse2+, ss+, htt+, tm+, ia64-, pbe+
      features ecx:                            sse3+, pclmulqdq+, dtes64-, monitor+, ds-cpl-, vmx-, smx-, est+, 
                                               tm2+, ssse3+, cntx-id-, sdbg-, fma+, cx16+, xtpr+, pdcm+, 
                                               pcid+, dca-, sse4.1+, sse4.2+, x2apic+, movbe+, popcnt+, 
                                               tsc-deadline+, aes+, xsave+, osxsave+, avx+, f16c+, rdrnd+, hypervisor+
      Intel core count:                        64
      brand:                                   12th Gen Intel(R) Core(TM) i7-1280P
      features 7/0 ebx:                        fsgsbase+, tsc_adj-, sgx-, bmi1+, hle-, avx2+, fdnex-, smep+, 
                                               bmi2+, erms+, invpcid+, rtm-, rtm-m-, mpx-, rdt-a-, 
                                               avx512-f-, avx512-dq-, rdseed+, adx+, smap+, avx512-ifma-, clflushopt+, 
                                               clwb+, pt+, avx512-pf-, avx512-er-, avx512-cd-, sha+, avx512-bw-, avx512-vl-
      features 7/0 ecx:                        prefetchwt1-, avx512-vbmi-, umip+, pku-, ospke-, waitpkg+, avx512-vbmi2-, cet-ss+, 
                                               gfni+, vaes+, vpclmulqdq+, avx512-vnni-, avx512-bitalg-, tme+, avx512-vpopcntdq-, 
                                               la57-, rdpid+, kl-, 
                                               bus-lock_detect-, cidemote-, movdiri+, movdir64b+, enqcmd-, sgx-lc-, pks-
      features 7/0 edx:                        avx512-4vnniw-, avx512-4fmaps-, fsrm+, uintr-, 
                                               avx512-vp2intersect-, srdbs-ctrl-, mc-clear+, rtmaa-, tsx_force_abort-, serialize+, hybrid+, 
                                               tsxldtrk-, pconfig-, lbr+, cet-ibt+, amx-bf16-, avx512-fp16-, 
                                               amx-tile-, amx-int8-, ibrs_ibpb+, stibp+, ld1_flush+, ia32_arch+, ia32_core-, ssbd+
      l2 cache in kb:
        size in kb:                            1280
        associativity:                         reserved16 ways
        lines per tag:                         0
        line size in bytes:                    64
      physical / linear address bits:          39 / 48
    processor info via registry key HKLM\HARDWARE\DESCRIPTION\System\CentralProcessor\0:
      Identifier:                              Intel64 Family 6 Model 154 Stepping 3
      ProcessorNameString:                     12th Gen Intel(R) Core(TM) i7-1280P
      VendorIdentifier:                        GenuineIntel
    bios info via registry key HKLM\HARDWARE\DESCRIPTION\System\BIOS:
      BaseBoardManufacturer:                   LENOVO
      BaseBoardProduct:                        21E8CTO1WW
      BaseBoardVersion:                        SDK0T76461 WIN
      BiosVendor:                              LENOVO
      BiosReleaseDate:                         10/06/2022
      BiosVersion:                             N3IET37W (1.17 )
      SystemFamily:                            ThinkPad X1 Nano Gen 2
      SystemManufacturer:                      LENOVO
      SystemProductName:                       21E8CTO1WW
      SystemVersion:                           ThinkPad X1 Nano Gen 2
    system memory info via GetPhysicallyInstalledSystemMemory:
      physically installed system memory:      32768 MB == 32 GB
      total usable physical memory:            32435 MB == 31 GB
    cpu speed via CallNtPowerInformation:
      Mhz current:                             1800
      Mhz max:                                 1800
      Mhz limit:                               1800
    system info via GetSystemInfo:
      processor architecture:                  x64
      page size:                               4096
      allocation granularity:                  65536
      processor level (family):                6
      processor revision (model):              0x9a03
      number of processors:                    20
    process and system architecture via IsWow64Process2 and GetProcessInformation:
      process / system architecture:           0x8664 == AMD 64 / 0x8664 == AMD 64
    network adapters via GetAdaptersInfo:
      adapter description:                     Bluetooth Device (Personal Area Network)
        adapter name:                          {A065205B-8574-48EB-A857-F013F2DABA3E}
        ipaddr / subnet mask / gateway / dhcp: 0.0.0.0 / 0.0.0.0 / 0.0.0.0 / true
        adapter (mac) address:                 28-6B-35-01-E6-44
      adapter description:                     Lenovo USB Ethernet #2
        adapter name:                          {41F20E6C-E7DE-4FA7-BECD-AE23E3FDB1A0}
        ipaddr / subnet mask / gateway / dhcp: 192.168.0.205 / 255.255.255.0 / 192.168.0.1 / true
        adapter (mac) address:                 60-7D-09-01-39-2B
      adapter description:                     Intel(R) Wi-Fi 6E AX211 160MHz
        adapter name:                          {1692939F-B98F-4CFF-8A1D-179258B2D310}
        ipaddr / subnet mask / gateway / dhcp: 0.0.0.0 / 0.0.0.0 / 0.0.0.0 / true
        adapter (mac) address:                 28-6B-35-01-E6-40
      adapter description:                     Microsoft Wi-Fi Direct Virtual Adapter
        adapter name:                          {5B80AA03-1BEF-4F87-880F-67E331D76EDE}
        ipaddr / subnet mask / gateway / dhcp: 0.0.0.0 / 0.0.0.0 / 0.0.0.0 / true
        adapter (mac) address:                 28-6B-35-01-E6-41
      adapter description:                     Microsoft Wi-Fi Direct Virtual Adapter #2
        adapter name:                          {0EEE64F0-3712-4D88-B7BD-C05A2EECFFD6}
        ipaddr / subnet mask / gateway / dhcp: 0.0.0.0 / 0.0.0.0 / 0.0.0.0 / true
        adapter (mac) address:                 2A-6B-35-01-E6-40
    os version via RtlGetVersion:
      major.minor.build:                       10.0.22621
      build # > 22000, so it's at least        Windows 11
      product type:                            0x65 == Windows 10 Home
    monitor information via EnumDisplayMonitors:
      monitor 0 rectangle:                     0, 0, 2160, 1350
    monitor information via GetIntegratedDisplaySize:
      internal display size in inches:         12.999580
    drive info via GetDriveType, GetDiskFreeSpace(Ex):
          type      fs                   volume        total         free
      c:  fixed   NTFS                  Windows      974484m      885944m
