# si
System Information. Command-line Windows app that shows info about the hardware and OS

Instructions for building are in the source file.

Sample output:

    computer name:                             DAVID-PC
    user name:                                 david
    processor info via GetLogicalProcessorInformationEx:
      L1 cache / line size:                    32k / 64
      L2 cache / line size:                    512k / 64
      L3 cache / line size:                    32768k / 64
      number of processor L1 / L2 / L3 caches: 32 / 16 / 2
      number of NUMA nodes:                    1
      number of physical processor packages:   1
      number of processor cores:               16
      number of logical processors:            32
    cpu information from cpuid:
      cpu make:                                AuthenticAMD
      highest cpuid / extended functions:      0xd / 0x80000021
      stepping id:                             0
      processor type:                          oem
      model / family:                          33 / 25
      hyper-threading available:               true
      legacy logical processor count:          32
      cache line size:                         64
      features edx:                            fpu+, vme+, de+, pse+, tsc+, msr+, pae+, mce+, 
                                               cx8+, apic+, sep+, mtrr+, pge+, mca+, cmov+, 
                                               pat+, pse-36+, psn-, clfsh+, ds-, acpi-, mmx+, 
                                               fxsr+, sse+, sse2+, ss-, htt+, tm-, ia64-, pbe-
      features ecx:                            sse3+, pclmulqdq+, dtes64-, monitor+, ds-cpl-, vmx-, smx-, est-, 
                                               tm2-, ssse3+, cntx-id-, sdbg-, fma+, cx16+, xtpr-, pdcm-, 
                                               pcid-, dca-, sse4.1+, sse4.2+, x2apic-, movbe+, popcnt+, 
                                               tsc-deadline-, aes+, xsave+, osxsave+, avx+, f16c+, rdrnd+, hypervisor+
      brand:                                   AMD Ryzen 9 5950X 16-Core Processor            
      features 7/0 ebx:                        fsgsbase+, tsc_adj-, sgx-, bmi1+, hle-, avx2+, fdnex-, smep+, 
                                               bmi2+, erms+, invpcid+, rtm-, rtm-m+, mpx-, rdt-a+, 
                                               avx512-f-, avx512-dq-, rdseed+, adx+, smap+, avx512-ifma-, clflushopt+, 
                                               clwb+, pt-, avx512-pf-, avx512-er-, avx512-cd-, sha+, avx512-bw-, avx512-vl-
      features 7/0 ecx:                        prefetchwt1-, avx512-vbmi-, umip+, pku-, ospke-, waitpkg-, avx512-vbmi2-, cet-ss+, 
                                               gfni-, vaes+, vpclmulqdq+, avx512-vnni-, avx512-bitalg-, tme-, avx512-vpopcntdq-, 
                                               la57-, rdpid+, kl-, 
                                               bus-lock_detect-, cidemote-, movdiri-, movdir64b-, enqcmd-, sgx-lc-, pks-
      features 7/0 edx:                        avx512-4vnniw-, avx512-4fmaps-, fsrm+, uintr-, 
                                               avx512-vp2intersect-, srdbs-ctrl-, mc-clear-, rtmaa-, tsx_force_abort-, serialize-, hybrid-, 
                                               tsxldtrk-, pconfig-, lbr-, cet-ibt-, amx-bf16-, avx512-fp16-, 
                                               amx-tile-, amx-int8-, ibrs_ibpb-, stibp-, ld1_flush-, ia32_arch-, ia32_core-, ssbd-
      AMD physical / logical cores:            32 / 64
      log2 of max apic id:                     5
      physical / linear address bits:          48 / 48
    system memory info via GetPhysicallyInstalledSystemMemory:
      physically installed system memory:      131072 MB == 128 GB
      total usable physical memory:            130971 MB == 127 GB
    cpu speed via CallNtPowerInformation:
      Mhz current:                             3401
      Mhz max:                                 3401
      Mhz limit:                               3401
    system info via GetSystemInfo:
      processor architecture:                  x64
      page size:                               4096
      allocation granularity:                  65536
      processor level (family):                25
      processor revision (model):              0x2100
      number of processors:                    32
    network adapters via GetAdaptersInfo:
      adapter description:                     Bluetooth Device (Personal Area Network)
        adapter name:                          {2579DECA-D592-4B6B-9FBD-275355C40C96}
        ip address / subnet mask / gateway:    0.0.0.0 / 0.0.0.0 / 0.0.0.0
        dhcp enabled:                          true
        adapter (mac) address:                 A8-7E-EA-E9-E7-23
      adapter description:                     Hyper-V Virtual Ethernet Adapter
        adapter name:                          {F137154C-EA54-4D13-B6B6-8BC156B7F5A2}
        ip address / subnet mask / gateway:    172.25.224.1 / 255.255.240.0 / 0.0.0.0
        dhcp enabled:                          false
        adapter (mac) address:                 00-15-5D-80-E9-7D
      adapter description:                     Hyper-V Virtual Ethernet Adapter #2
        adapter name:                          {B63EFA74-4992-4635-9EC1-C2E3187E1965}
        ip address / subnet mask / gateway:    192.168.0.105 / 255.255.255.0 / 192.168.0.1
        dhcp enabled:                          true
        adapter (mac) address:                 18-C0-4D-60-86-41
    os version via RtlGetVersion:
      major.minor.build:                       10.0.22621
      build # > 22000, so it's at least        Windows 11
      product type:                            0x30 == Windows 10 Pro
    monitor information via EnumDisplayMonitors:
      monitor 0 rectangle:                     0, 0, 3072, 1728
      monitor 1 rectangle:                     -3072, 0, 0, 1728
    drive info via GetDriveType, GetDiskFreeSpace(Ex):
          type      fs                   volume        total         free
      c:  fixed   NTFS                   c_boot     1897370m     1230837m
      d:  fixed   NTFS               d_ssd_24tb    22892603m    14869926m
      g:  remov      ?                         
      s:  fixed   NTFS            s_ssd_4tb_pci     3815453m     3632851m
      y:  fixed   NTFS          y_wd_12tb_dcopy    11444093m     3363870m
      z:  fixed   NTFS            z_4tb_ssd_far     3815453m      320157m
