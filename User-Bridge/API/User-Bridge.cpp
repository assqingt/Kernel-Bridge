#include <Windows.h>

#include "WdkTypes.h"
#include "CtlTypes.h"
#include "User-Bridge.h"

#include "DriversUtils.h"


namespace KbLoader {

    constexpr LPCWSTR KbDriverName = L"Kernel-Bridge";
    constexpr LPCWSTR KbDeviceName = L"\\\\.\\Kernel-Bridge";

    static HANDLE hDriver = INVALID_HANDLE_VALUE;

    BOOL WINAPI KbLoadAsDriver(LPCWSTR DriverPath)
    {
        // Check whether the Kernel-Bridge is already loaded:
        if (hDriver != INVALID_HANDLE_VALUE) return TRUE;
        hDriver = OpenDevice(KbDeviceName);
        if (hDriver != INVALID_HANDLE_VALUE) return TRUE;

        // Installing driver:
        BOOL Status = InstallDriver(DriverPath, KbDriverName);
        if (!Status) return FALSE;

        // Obtaining it's handle:
        hDriver = OpenDevice(KbDeviceName);
        if (hDriver == INVALID_HANDLE_VALUE) {
            DeleteDriver(KbDriverName);
            return FALSE;
        }

        return TRUE;
    }

    BOOL WINAPI KbLoadAsFilter(
        LPCWSTR DriverPath,
        LPCWSTR Altitude
    ) {
        // Check whether the Kernel-Bridge is already loaded:
        if (hDriver != INVALID_HANDLE_VALUE) return TRUE;
        hDriver = OpenDevice(KbDeviceName);
        if (hDriver != INVALID_HANDLE_VALUE) return TRUE;

        BOOL Status = InstallMinifilter(DriverPath, KbDriverName, Altitude);
        if (!Status) return FALSE;

        // Obtaining it's handle:
        hDriver = OpenDevice(KbDeviceName);
        if (hDriver == INVALID_HANDLE_VALUE) {
            DeleteDriver(KbDriverName);
            return FALSE;
        }

        return TRUE;
    }

    BOOL WINAPI KbUnload()
    {
        if (hDriver == INVALID_HANDLE_VALUE) return TRUE;
        CloseHandle(hDriver);
        return DeleteDriver(KbDriverName);
    }
}

static inline BOOL WINAPI KbSendRequest(
    Ctls::KbCtlIndices Index, 
    IN PVOID Input = NULL, 
    ULONG InputSize = 0, 
    OUT PVOID Output = NULL, 
    ULONG OutputSize = 0
) {
    return SendIOCTL(KbLoader::hDriver, CTL_BASE + Index, Input, InputSize, Output, OutputSize);
}

namespace IO {
    namespace Beeper {
        BOOL WINAPI KbSetBeeperRegime() {
            return KbSendRequest(Ctls::KbSetBeeperRegime);
        }

        BOOL WINAPI KbStartBeeper() {
            return KbSendRequest(Ctls::KbStartBeeper);
        }

        BOOL WINAPI KbStopBeeper() {
            return KbSendRequest(Ctls::KbStopBeeper);
        }

        BOOL WINAPI KbSetBeeperIn() {
            return KbSendRequest(Ctls::KbSetBeeperIn);
        }

        BOOL WINAPI KbSetBeeperOut() {
            return KbSendRequest(Ctls::KbSetBeeperOut);
        }

        BOOL WINAPI KbSetBeeperDivider(USHORT Divider) {
            KB_SET_BEEPER_DIVIDER_IN Input = {};
            Input.Divider = Divider;
            return KbSendRequest(Ctls::KbSetBeeperDivider, &Input, sizeof(Input));
        }

        BOOL WINAPI KbSetBeeperFrequency(USHORT Frequency) {
            KB_SET_BEEPER_FREQUENCY_IN Input = {};
            Input.Frequency = Frequency;
            return KbSendRequest(Ctls::KbSetBeeperFrequency, &Input, sizeof(Input));
        }
    }

    namespace RW {
        BOOL WINAPI KbReadPortByte(USHORT PortNumber, OUT PUCHAR Value) {
            if (!Value) return FALSE;
            KB_READ_PORT_IN Input = {};
            KB_READ_PORT_BYTE_OUT Output = {};
            Input.PortNumber = PortNumber;
            BOOLEAN Status = KbSendRequest(Ctls::KbReadPort, &Input, sizeof(Input), &Output, sizeof(Output));
            *Value = Output.Value;
            return Status;
        }

        BOOL WINAPI KbReadPortWord(USHORT PortNumber, OUT PUSHORT Value) {
            if (!Value) return FALSE;
            KB_READ_PORT_IN Input = {};
            KB_READ_PORT_WORD_OUT Output = {};
            Input.PortNumber = PortNumber;
            BOOLEAN Status = KbSendRequest(Ctls::KbReadPort, &Input, sizeof(Input), &Output, sizeof(Output));
            *Value = Output.Value;
            return Status;
        }

        BOOL WINAPI KbReadPortDword(USHORT PortNumber, OUT PULONG Value) {
            if (!Value) return FALSE;
            KB_READ_PORT_IN Input = {};
            KB_READ_PORT_DWORD_OUT Output = {};
            Input.PortNumber = PortNumber;
            BOOLEAN Status = KbSendRequest(Ctls::KbReadPort, &Input, sizeof(Input), &Output, sizeof(Output));
            *Value = Output.Value;
            return Status;
        }

        BOOL WINAPI KbReadPortByteString(USHORT PortNumber, ULONG Count, OUT PUCHAR ByteString, ULONG ByteStringSizeInBytes) {
            if (!Count || !ByteString || !ByteStringSizeInBytes) return FALSE;
            KB_READ_PORT_STRING_IN Input = {};
            auto Output = reinterpret_cast<PKB_READ_PORT_STRING_OUT>(ByteString);
            Input.PortNumber = PortNumber;
            return KbSendRequest(Ctls::KbReadPortString, &Input, sizeof(Input), Output, ByteStringSizeInBytes);
        }

        BOOL WINAPI KbReadPortWordString(USHORT PortNumber, ULONG Count, OUT PUSHORT WordString, ULONG WordStringSizeInBytes) {
            if (!Count || !WordString || !WordStringSizeInBytes) return FALSE;
            KB_READ_PORT_STRING_IN Input = {};
            auto Output = reinterpret_cast<PKB_READ_PORT_STRING_OUT>(WordString);
            Input.PortNumber = PortNumber;
            return KbSendRequest(Ctls::KbReadPortString, &Input, sizeof(Input), Output, WordStringSizeInBytes);
        }

        BOOL WINAPI KbReadPortDwordString(USHORT PortNumber, ULONG Count, OUT PULONG DwordString, ULONG DwordStringSizeInBytes) {
            if (!Count || !DwordString || !DwordStringSizeInBytes) return FALSE;
            KB_READ_PORT_STRING_IN Input = {};
            auto Output = reinterpret_cast<PKB_READ_PORT_STRING_OUT>(DwordString);
            Input.PortNumber = PortNumber;
            return KbSendRequest(Ctls::KbReadPortString, &Input, sizeof(Input), Output, DwordStringSizeInBytes);
        }

        BOOL WINAPI KbWritePortByte(USHORT PortNumber, UCHAR Value) {
            KB_WRITE_PORT_IN Input = {};
            Input.PortNumber = PortNumber;
            Input.Granularity = sizeof(Value);
            Input.Byte = Value;
            return KbSendRequest(Ctls::KbWritePort, &Input, sizeof(Input));
        }

        BOOL WINAPI KbWritePortWord(USHORT PortNumber, USHORT Value) {
            KB_WRITE_PORT_IN Input = {};
            Input.PortNumber = PortNumber;
            Input.Granularity = sizeof(Value);
            Input.Word = Value;
            return KbSendRequest(Ctls::KbWritePort, &Input, sizeof(Input));
        }

        BOOL WINAPI KbWritePortDword(USHORT PortNumber, ULONG Value) {
            KB_WRITE_PORT_IN Input = {};
            Input.PortNumber = PortNumber;
            Input.Granularity = sizeof(Value);
            Input.Dword = Value;
            return KbSendRequest(Ctls::KbWritePort, &Input, sizeof(Input));
        }

        BOOL WINAPI KbWritePortByteString(USHORT PortNumber, ULONG Count, IN PUCHAR ByteString, ULONG ByteStringSizeInBytes) {
            if (!ByteString || ByteStringSizeInBytes < Count * sizeof(*ByteString)) return FALSE;
            KB_WRITE_PORT_STRING_IN Input = {};
            Input.PortNumber = PortNumber;
            Input.Granularity = sizeof(*ByteString);
            Input.Count = Count;
            Input.BufferSize = ByteStringSizeInBytes;
            Input.Buffer = reinterpret_cast<WdkTypes::PVOID>(ByteString);
            return KbSendRequest(Ctls::KbWritePortString, &Input, sizeof(Input));
        }

        BOOL WINAPI KbWritePortWordString(USHORT PortNumber, ULONG Count, IN PUSHORT WordString, ULONG WordStringSizeInBytes) {
            if (!WordString || WordStringSizeInBytes < Count * sizeof(*WordString)) return FALSE;
            KB_WRITE_PORT_STRING_IN Input = {};
            Input.PortNumber = PortNumber;
            Input.Granularity = sizeof(*WordString);
            Input.Count = Count;
            Input.BufferSize = WordStringSizeInBytes;
            Input.Buffer = reinterpret_cast<WdkTypes::PVOID>(WordString);
            return KbSendRequest(Ctls::KbWritePortString, &Input, sizeof(Input));
        }

        BOOL WINAPI KbWritePortDwordString(USHORT PortNumber, ULONG Count, IN PULONG DwordString, ULONG DwordStringSizeInBytes) {
            if (!DwordString || DwordStringSizeInBytes < Count * sizeof(*DwordString)) return FALSE;
            KB_WRITE_PORT_STRING_IN Input = {};
            Input.PortNumber = PortNumber;
            Input.Granularity = sizeof(*DwordString);
            Input.Count = Count;
            Input.BufferSize = DwordStringSizeInBytes;
            Input.Buffer = reinterpret_cast<WdkTypes::PVOID>(DwordString);
            return KbSendRequest(Ctls::KbWritePortString, &Input, sizeof(Input));
        }
    }

    namespace Iopl {
        BOOL WINAPI KbRaiseIopl() {
            return KbSendRequest(Ctls::KbRaiseIopl);
        }

        BOOL WINAPI KbResetIopl() {
            return KbSendRequest(Ctls::KbResetIopl);
        }    
    }
}

namespace CPU {
    BOOL WINAPI KbCli() {
        return KbSendRequest(Ctls::KbCli);
    }

    BOOL WINAPI KbSti() {
        return KbSendRequest(Ctls::KbSti);
    }    

    BOOL WINAPI KbHlt() {
        return KbSendRequest(Ctls::KbHlt);
    }    

    BOOL WINAPI KbReadMsr(ULONG Index, OUT PUINT64 MsrValue) {
        if (!MsrValue) return FALSE;
        KB_READ_MSR_IN Input = {};
        KB_READ_MSR_OUT Output = {};
        Input.Index = Index;
        BOOL Status = KbSendRequest(Ctls::KbReadMsr, &Input, sizeof(Input), &Output, sizeof(Output));
        *MsrValue = Output.Value;
        return Status;
    }

    BOOL WINAPI KbWriteMsr(ULONG Index, IN UINT64 MsrValue) {
        KB_WRITE_MSR_IN Input = {};
        Input.Index = Index;
        Input.Value = MsrValue;
        return KbSendRequest(Ctls::KbWriteMsr, &Input, sizeof(Input));
    }

    BOOL WINAPI KbCpuid(ULONG FunctionIdEax, OUT PCPUID_INFO CpuidInfo) {
        if (!CpuidInfo) return FALSE;
        KB_CPUID_IN Input = {};
        KB_CPUID_OUT Output = {};
        Input.FunctionIdEax = FunctionIdEax;
        BOOL Status = KbSendRequest(Ctls::KbCpuid, &Input, sizeof(Input), &Output, sizeof(Output));
        CpuidInfo->Eax = Output.Eax;
        CpuidInfo->Ebx = Output.Ebx;
        CpuidInfo->Ecx = Output.Ecx;
        CpuidInfo->Edx = Output.Edx;
        return Status;
    }

    BOOL WINAPI KbCpuidEx(ULONG FunctionIdEax, ULONG SubfunctionIdEcx, OUT PCPUID_INFO CpuidInfo) {
        if (!CpuidInfo) return FALSE;
        KB_CPUIDEX_IN Input = {};
        KB_CPUID_OUT Output = {};
        Input.FunctionIdEax = FunctionIdEax;
        Input.SubfunctionIdEcx = SubfunctionIdEcx;
        BOOL Status = KbSendRequest(Ctls::KbCpuidEx, &Input, sizeof(Input), &Output, sizeof(Output));
        CpuidInfo->Eax = Output.Eax;
        CpuidInfo->Ebx = Output.Ebx;
        CpuidInfo->Ecx = Output.Ecx;
        CpuidInfo->Edx = Output.Edx;
        return Status;
    }

    BOOL WINAPI KbReadPmc(ULONG Counter, OUT PUINT64 PmcValue) {
        if (!PmcValue) return FALSE;
        KB_READ_PMC_IN Input = {};
        KB_READ_PMC_OUT Output = {};
        Input.Counter = Counter;
        BOOL Status = KbSendRequest(Ctls::KbReadPmc, &Input, sizeof(Input), &Output, sizeof(Output));
        *PmcValue = Output.Value;
        return Status;
    }

    BOOL WINAPI KbReadTsc(OUT PUINT64 TscValue) {
        if (!TscValue) return FALSE;
        KB_READ_TSC_OUT Output = {};
        BOOL Status = KbSendRequest(Ctls::KbReadTsc, NULL, 0, &Output, sizeof(Output));
        *TscValue = Output.Value;
        return Status;
    }

    BOOL WINAPI KbReadTscp(OUT PUINT64 TscValue, OUT OPTIONAL PULONG TscAux) {
        if (!TscValue) return FALSE;
        KB_READ_TSCP_OUT Output = {};
        BOOL Status = KbSendRequest(Ctls::KbReadTscp, NULL, 0, &Output, sizeof(Output));
        *TscValue = Output.Value;
        if (TscAux) *TscAux = Output.TscAux;
        return Status;
    }
}

namespace VirtualMemory {
    BOOL WINAPI KbAllocKernelMemory(ULONG Size, BOOLEAN Executable, OUT WdkTypes::PVOID* KernelAddress) {
        if (!Size || !KernelAddress) return FALSE;
        KB_ALLOC_KERNEL_MEMORY_IN Input = {};
        KB_ALLOC_KERNEL_MEMORY_OUT Output = {};
        Input.Size = Size;
        Input.Executable = Executable;
        BOOL Status = KbSendRequest(Ctls::KbAllocKernelMemory, &Input, sizeof(Input), &Output, sizeof(Output));
        *KernelAddress = Output.KernelAddress;
        return Status;
    }

    BOOL WINAPI KbFreeKernelMemory(IN WdkTypes::PVOID KernelAddress) {
        if (!KernelAddress) return FALSE;
        KB_FREE_KERNEL_MEMORY_IN Input = {};
        Input.KernelAddress = KernelAddress;
        BOOL Status = KbSendRequest(Ctls::KbFreeKernelMemory, &Input, sizeof(Input));
        DWORD LE = GetLastError();
        return Status;
    }

    BOOL WINAPI KbCopyMoveMemory(OUT WdkTypes::PVOID Dest, IN WdkTypes::PVOID Src, ULONG Size, BOOLEAN Intersects) {
        if (!Dest || !Src || !Size) return FALSE;
        KB_COPY_MOVE_MEMORY_IN Input = {};
        Input.Src = Src;
        Input.Dest = Dest;
        Input.Size = Size;
        Input.Intersects = Intersects;
        return KbSendRequest(Ctls::KbCopyMoveMemory, &Input, sizeof(Input));
    }

    BOOL WINAPI KbFillMemory(IN WdkTypes::PVOID Address, UCHAR Filler, ULONG Size) {
        if (!Address || !Size) return FALSE;
        KB_FILL_MEMORY_IN Input = {};
        Input.Address = Address;
        Input.Size = Size;
        Input.Filler = Filler;
        return KbSendRequest(Ctls::KbFillMemory, &Input, sizeof(Input));
    }

    BOOL WINAPI KbEqualMemory(IN WdkTypes::PVOID Src, IN WdkTypes::PVOID Dest, ULONG Size, OUT PBOOLEAN Equals) {
        if (!Src || !Dest || !Size || !Equals) return FALSE;
        KB_EQUAL_MEMORY_IN Input = {};
        KB_EQUAL_MEMORY_OUT Output = {};
        Input.Src = Src;
        Input.Dest = Dest;
        Input.Size = Size;
        BOOL Status = KbSendRequest(Ctls::KbEqualMemory, &Input, sizeof(Input), &Output, sizeof(Output));
        *Equals = Output.Equals;
        return Status;
    }
}

namespace Mdl {
    BOOL WINAPI KbMapMdl(
        OUT WdkTypes::PVOID* MappedMemory,
        OPTIONAL UINT64 SrcProcessId,
        OPTIONAL UINT64 DestProcessId,
        WdkTypes::PMDL Mdl,
        BOOLEAN NeedLock,
        WdkTypes::KPROCESSOR_MODE AccessMode,
        ULONG Protect,
        WdkTypes::MEMORY_CACHING_TYPE CacheType,
        OPTIONAL WdkTypes::PVOID UserRequestedAddress
    ) {
        if (!MappedMemory) return FALSE;
        KB_MAP_MDL_IN Input = {};
        KB_MAP_MDL_OUT Output = {};
        Input.SrcProcessId = SrcProcessId;
        Input.DestProcessId = DestProcessId;
        Input.Mdl = Mdl;
        Input.NeedLock = NeedLock;
        Input.AccessMode = AccessMode;
        Input.Protect = Protect;
        Input.CacheType = CacheType;
        Input.UserRequestedAddress;
        BOOL Status = KbSendRequest(Ctls::KbMapMdl, &Input, sizeof(Input), &Output, sizeof(Output));
        *MappedMemory = Output.BaseAddress;
        return Status;
    }

    BOOL WINAPI KbMapMemory(
        OUT PMAPPING_INFO MappingInfo,
        OPTIONAL UINT64 SrcProcessId,
        OPTIONAL UINT64 DestProcessId,
        WdkTypes::PVOID VirtualAddress,
        ULONG Size,
        WdkTypes::KPROCESSOR_MODE AccessMode,
        ULONG Protect,
        WdkTypes::MEMORY_CACHING_TYPE CacheType,
        OPTIONAL WdkTypes::PVOID UserRequestedAddress
    ) {
        if (!MappingInfo || !Size) return FALSE;
        KB_MAP_MEMORY_IN Input = {};
        KB_MAP_MEMORY_OUT Output = {};
        Input.SrcProcessId = SrcProcessId;
        Input.DestProcessId = DestProcessId;
        Input.VirtualAddress = VirtualAddress;
        Input.Size = Size;
        Input.AccessMode = AccessMode;
        Input.Protect = Protect;
        Input.CacheType = CacheType;
        Input.UserRequestedAddress;
        BOOL Status = KbSendRequest(Ctls::KbMapMemory, &Input, sizeof(Input), &Output, sizeof(Output));
        MappingInfo->MappedAddress = Output.BaseAddress;
        MappingInfo->Mdl = Output.Mdl;
        return Status;
    }

    BOOL WINAPI KbProtectMappedMemory(IN WdkTypes::PMDL Mdl, ULONG Protect) {
        if (!Mdl) return FALSE;
        KB_PROTECT_MAPPED_MEMORY_IN Input = {};
        Input.Mdl = Mdl;
        Input.Protect = Protect;
        return KbSendRequest(Ctls::KbProtectMappedMemory, &Input, sizeof(Input));
    }

    BOOL WINAPI KbUnmapMdl(IN WdkTypes::PMDL Mdl, IN WdkTypes::PVOID MappedMemory, BOOLEAN NeedUnlock) {
        if (!Mdl || !MappedMemory) return FALSE;
        KB_UNMAP_MDL_IN Input = {};
        Input.Mdl = Mdl;
        Input.BaseAddress = MappedMemory;
        Input.NeedUnlock = NeedUnlock;
        return KbSendRequest(Ctls::KbUnmapMdl, &Input, sizeof(Input));
    }

    BOOL WINAPI KbUnmapMemory(IN PMAPPING_INFO MappingInfo) {
        if (!MappingInfo || !MappingInfo->Mdl) return FALSE;
        KB_UNMAP_MEMORY_IN Input = {};
        Input.Mdl = MappingInfo->Mdl;
        Input.BaseAddress = MappingInfo->MappedAddress;
        return KbSendRequest(Ctls::KbUnmapMemory, &Input, sizeof(Input));
    }
}

namespace PhysicalMemory {
    BOOL WINAPI KbMapPhysicalMemory(IN WdkTypes::PVOID PhysicalAddress, ULONG Size, OUT WdkTypes::PVOID* VirtualAddress) {
        if (!Size || !VirtualAddress) return FALSE;
        KB_MAP_PHYSICAL_MEMORY_IN Input = {};
        KB_MAP_PHYSICAL_MEMORY_OUT Output = {};
        Input.PhysicalAddress = PhysicalAddress;
        Input.Size = Size;
        BOOL Status = KbSendRequest(Ctls::KbMapPhysicalMemory, &Input, sizeof(Input), &Output, sizeof(Output));
        *VirtualAddress = Output.VirtualAddress;
        return Status;
    }

    BOOL WINAPI KbUnmapPhysicalMemory(IN WdkTypes::PVOID VirtualAddress, ULONG Size) {
        if (!VirtualAddress || !Size) return FALSE;
        KB_UNMAP_PHYSICAL_MEMORY_IN Input = {};
        Input.VirtualAddress = VirtualAddress;
        Input.Size = Size;
        return KbSendRequest(Ctls::KbUnmapPhysicalMemory, &Input, sizeof(Input));
    }

    BOOL WINAPI KbGetPhysicalAddress(
        IN OPTIONAL WdkTypes::PEPROCESS Process, 
        IN WdkTypes::PVOID VirtualAddress,
        OUT WdkTypes::PVOID* PhysicalAddress
    ) {
        if (!PhysicalAddress) return FALSE;
        KB_GET_PHYSICAL_ADDRESS_IN Input = {};
        KB_GET_PHYSICAL_ADDRESS_OUT Output = {};
        Input.Process = Process;
        Input.VirtualAddress = VirtualAddress;
        BOOL Status = KbSendRequest(Ctls::KbGetPhysicalAddress, &Input, sizeof(Input), &Output, sizeof(Output));
        *PhysicalAddress = Output.PhysicalAddress;
        return Status;
    }

    BOOL WINAPI KbReadPhysicalMemory(
        WdkTypes::PVOID64 PhysicalAddress,
        OUT PVOID Buffer,
        ULONG Size
    ) {
        if (!Buffer || !Size) return FALSE;
        KB_READ_WRITE_PHYSICAL_MEMORY_IN Input = {};
        Input.PhysicalAddress = PhysicalAddress;
        Input.Buffer = reinterpret_cast<WdkTypes::PVOID>(Buffer);
        Input.Size = Size;
        return KbSendRequest(Ctls::KbReadPhysicalMemory, &Input, sizeof(Input));
    }

    BOOL WINAPI KbWritePhysicalMemory(
        WdkTypes::PVOID64 PhysicalAddress,
        IN PVOID Buffer,
        ULONG Size
    ) {
        if (!Buffer || !Size) return FALSE;
        KB_READ_WRITE_PHYSICAL_MEMORY_IN Input = {};
        Input.PhysicalAddress = PhysicalAddress;
        Input.Buffer = reinterpret_cast<WdkTypes::PVOID>(Buffer);
        Input.Size = Size;
        return KbSendRequest(Ctls::KbWritePhysicalMemory, &Input, sizeof(Input));
    }

    BOOL WINAPI KbReadDmiMemory(OUT UCHAR DmiMemory[DmiSize], ULONG BufferSize) {
        if (BufferSize != DmiSize) return FALSE;
        return KbSendRequest(Ctls::KbReadDmiMemory, NULL, 0, reinterpret_cast<PKB_READ_DMI_MEMORY_OUT>(DmiMemory), BufferSize);
    }
}

namespace Processes {
    namespace Descriptors {
        BOOL WINAPI KbGetEprocess(ULONG ProcessId, OUT WdkTypes::PEPROCESS* Process) {
            if (!Process) return FALSE;
            KB_GET_EPROCESS_IN Input = {};
            KB_GET_EPROCESS_OUT Output = {};
            Input.ProcessId = ProcessId;
            BOOL Status = KbSendRequest(Ctls::KbGetEprocess, &Input, sizeof(Input), &Output, sizeof(Output));
            *Process = Output.Process;
            return Status;
        }

        BOOL WINAPI KbGetEthread(ULONG ThreadId, OUT WdkTypes::PETHREAD* Thread) {
            if (!Thread) return FALSE;
            KB_GET_ETHREAD_IN Input = {};
            KB_GET_ETHREAD_OUT Output = {};
            Input.ThreadId = ThreadId;
            BOOL Status = KbSendRequest(Ctls::KbGetEthread, &Input, sizeof(Input), &Output, sizeof(Output));
            *Thread = Output.Thread;
            return Status;
        }

        BOOL WINAPI KbOpenProcess(ULONG ProcessId, OUT WdkTypes::HANDLE* hProcess, OPTIONAL ACCESS_MASK Access, OPTIONAL ULONG Attributes) {
            if (!hProcess) return FALSE;
            KB_OPEN_PROCESS_IN Input = {};
            KB_OPEN_PROCESS_OUT Output = {};
            Input.ProcessId = ProcessId;
            Input.Access = Access;
            Input.Attributes = Attributes;
            BOOL Status = KbSendRequest(Ctls::KbOpenProcess, &Input, sizeof(Input), &Output, sizeof(Output));
            *hProcess = Output.hProcess;
            return Status;
        }

        BOOL WINAPI KbOpenProcessByPointer(
            WdkTypes::PEPROCESS Process, 
            OUT WdkTypes::HANDLE* hProcess, 
            OPTIONAL ACCESS_MASK Access, 
            OPTIONAL ULONG Attributes,
            OPTIONAL WdkTypes::KPROCESSOR_MODE ProcessorMode
        ) {
            if (!hProcess) return FALSE;
            KB_OPEN_PROCESS_BY_POINTER_IN Input = {};
            KB_OPEN_PROCESS_OUT Output = {};
            Input.Process = Process;
            Input.Access = Access;
            Input.Attributes = Attributes;
            Input.ProcessorMode = ProcessorMode;
            BOOL Status = KbSendRequest(Ctls::KbOpenProcessByPointer, &Input, sizeof(Input), &Output, sizeof(Output));
            *hProcess = Output.hProcess;
            return Status;
        }

        BOOL WINAPI KbOpenThread(ULONG ThreadId, OUT WdkTypes::HANDLE* hThread, OPTIONAL ACCESS_MASK Access, OPTIONAL ULONG Attributes) {
            if (!hThread) return FALSE;
            KB_OPEN_THREAD_IN Input = {};
            KB_OPEN_THREAD_OUT Output = {};
            Input.ThreadId = ThreadId;
            Input.Access = Access;
            Input.Attributes = Attributes;
            BOOL Status = KbSendRequest(Ctls::KbOpenThread, &Input, sizeof(Input), &Output, sizeof(Output));
            *hThread = Output.hThread;
            return Status;
        }

        BOOL WINAPI KbOpenThreadByPointer(
            WdkTypes::PETHREAD Thread, 
            OUT WdkTypes::HANDLE* hThread, 
            OPTIONAL ACCESS_MASK Access, 
            OPTIONAL ULONG Attributes,
            OPTIONAL WdkTypes::KPROCESSOR_MODE ProcessorMode
        ) {
            if (!hThread) return FALSE;
            KB_OPEN_THREAD_BY_POINTER_IN Input = {};
            KB_OPEN_THREAD_OUT Output = {};
            Input.Thread = Thread;
            Input.Access = Access;
            Input.Attributes = Attributes;
            Input.ProcessorMode = ProcessorMode;
            BOOL Status = KbSendRequest(Ctls::KbOpenThreadByPointer, &Input, sizeof(Input), &Output, sizeof(Output));
            *hThread = Output.hThread;
            return Status;
        }

        BOOL WINAPI KbDereferenceObject(WdkTypes::PVOID Object) {
            if (!Object) return FALSE;
            KB_DEREFERENCE_OBJECT_IN Input = {};
            Input.Object = Object;
            return KbSendRequest(Ctls::KbDereferenceObject, &Input, sizeof(Input));
        }

        BOOL WINAPI KbCloseHandle(WdkTypes::HANDLE Handle) {
            if (!Handle) return FALSE;
            KB_CLOSE_HANDLE_IN Input = {};
            Input.Handle = Handle;
            return KbSendRequest(Ctls::KbCloseHandle, &Input, sizeof(Input));
        }
    }

    namespace Threads {
        BOOL WINAPI KbCreateUserThread(
            ULONG ProcessId, 
            WdkTypes::PVOID ThreadRoutine, 
            WdkTypes::PVOID Argument, 
            BOOL CreateSuspended,
            OUT OPTIONAL WdkTypes::CLIENT_ID* ClientId,
            OUT OPTIONAL WdkTypes::HANDLE* hThread
        ) {
            if (!ProcessId) return FALSE;
            KB_CREATE_USER_THREAD_IN Input = {};
            KB_CREATE_USER_SYSTEM_THREAD_OUT Output = {};
            Input.ProcessId = ProcessId;
            Input.ThreadRoutine = ThreadRoutine;
            Input.Argument = Argument;
            Input.CreateSuspended = CreateSuspended;
            BOOL Status = KbSendRequest(Ctls::KbCreateUserThread, &Input, sizeof(Input), &Output, sizeof(Output));
            if (ClientId) *ClientId = Output.ClientId;
            if (*hThread) 
                *hThread = Output.hThread;
            else
                Descriptors::KbCloseHandle(Output.hThread);
            return Status;
        }

        BOOL WINAPI KbCreateSystemThread(
            ULONG ProcessId, 
            WdkTypes::PVOID ThreadRoutine, 
            WdkTypes::PVOID Argument,
            OUT OPTIONAL WdkTypes::CLIENT_ID* ClientId,
            OUT OPTIONAL WdkTypes::HANDLE* hThread
        ) {
            if (!ProcessId) return FALSE;
            KB_CREATE_SYSTEM_THREAD_IN Input = {};
            KB_CREATE_USER_SYSTEM_THREAD_OUT Output = {};
            Input.AssociatedProcessId = ProcessId;
            Input.ThreadRoutine = ThreadRoutine;
            Input.Argument = Argument;
            BOOL Status = KbSendRequest(Ctls::KbCreateSystemThread, &Input, sizeof(Input), &Output, sizeof(Output));
            if (ClientId) *ClientId = Output.ClientId;
            if (*hThread) 
                *hThread = Output.hThread;
            else
                Descriptors::KbCloseHandle(Output.hThread);
            return Status;
        }

        BOOL WINAPI KbSuspendProcess(ULONG ProcessId) {
            KB_SUSPEND_RESUME_PROCESS_IN Input = {};
            Input.ProcessId = ProcessId;
            return KbSendRequest(Ctls::KbSuspendProcess, &Input, sizeof(Input));
        }

        BOOL WINAPI KbResumeProcess(ULONG ProcessId) {
            KB_SUSPEND_RESUME_PROCESS_IN Input = {};
            Input.ProcessId = ProcessId;
            return KbSendRequest(Ctls::KbResumeProcess, &Input, sizeof(Input));
        }

        BOOL WINAPI KbGetThreadContext(ULONG ThreadId, OUT PCONTEXT Context, ULONG ContextSize, OPTIONAL WdkTypes::KPROCESSOR_MODE ProcessorMode) {
            KB_GET_SET_THREAD_CONTEXT_IN Input = {};
            Input.ThreadId = ThreadId;
            Input.ContextSize = ContextSize;
            Input.ProcessorMode = ProcessorMode;
            Input.Context = reinterpret_cast<WdkTypes::PVOID>(Context);
            return KbSendRequest(Ctls::KbGetThreadContext, &Input, sizeof(Input));
        }

        BOOL WINAPI KbSetThreadContext(ULONG ThreadId, IN PCONTEXT Context, ULONG ContextSize, OPTIONAL WdkTypes::KPROCESSOR_MODE ProcessorMode) {
            KB_GET_SET_THREAD_CONTEXT_IN Input = {};
            Input.ThreadId = ThreadId;
            Input.ContextSize = ContextSize;
            Input.ProcessorMode = ProcessorMode;
            Input.Context = reinterpret_cast<WdkTypes::PVOID>(Context);
            return KbSendRequest(Ctls::KbSetThreadContext, &Input, sizeof(Input));
        }
    }

    namespace MemoryManagement {
        BOOL WINAPI KbAllocUserMemory(ULONG ProcessId, ULONG Protect, ULONG Size, OUT WdkTypes::PVOID* BaseAddress) {
            if (!ProcessId || !Size || !BaseAddress) return FALSE;
            KB_ALLOC_USER_MEMORY_IN Input = {};
            KB_ALLOC_USER_MEMORY_OUT Output = {};
            Input.ProcessId = ProcessId;
            Input.Protect = Protect;
            Input.Size = Size;
            BOOL Status = KbSendRequest(Ctls::KbAllocUserMemory, &Input, sizeof(Input), &Output, sizeof(Output));
            *BaseAddress = Output.BaseAddress;
            return Status;
        }

        BOOL WINAPI KbFreeUserMemory(ULONG ProcessId, WdkTypes::PVOID BaseAddress) {
            if (!ProcessId || !BaseAddress) return FALSE;
            KB_FREE_USER_MEMORY_IN Input = {};
            Input.ProcessId = ProcessId;
            Input.BaseAddress = BaseAddress;
            return KbSendRequest(Ctls::KbFreeUserMemory, &Input, sizeof(Input));
        }

        BOOL WINAPI KbSecureVirtualMemory(
            ULONG ProcessId,
            WdkTypes::PVOID BaseAddress,
            ULONG Size,
            ULONG ProtectRights,
            OUT WdkTypes::HANDLE* SecureHandle
        ) {
            if (!ProcessId || !BaseAddress || !Size || !SecureHandle) return FALSE;
            KB_SECURE_VIRTUAL_MEMORY_IN Input = {};
            KB_SECURE_VIRTUAL_MEMORY_OUT Output = {};
            Input.ProcessId = ProcessId;
            Input.ProtectRights = ProtectRights;
            Input.BaseAddress = BaseAddress;
            Input.Size = Size;
            BOOL Status = KbSendRequest(Ctls::KbSecureVirtualMemory, &Input, sizeof(Input), &Output, sizeof(Output));
            *SecureHandle = Output.SecureHandle;
            return Status;
        }

        BOOL WINAPI KbUnsecureVirtualMemory(
            ULONG ProcessId,
            WdkTypes::HANDLE SecureHandle
        ) {
            if (!ProcessId || !SecureHandle) return FALSE;
            KB_UNSECURE_VIRTUAL_MEMORY_IN Input = {};
            Input.ProcessId = ProcessId;
            Input.SecureHandle = SecureHandle;
            return KbSendRequest(Ctls::KbUnsecureVirtualMemory, &Input, sizeof(Input));
        }

        BOOL WINAPI KbReadProcessMemory(ULONG ProcessId, IN WdkTypes::PVOID BaseAddress, OUT PVOID Buffer, ULONG Size) {
            if (!ProcessId || !BaseAddress || !Buffer || !Size) return FALSE;
            KB_READ_WRITE_PROCESS_MEMORY_IN Input = {};
            Input.ProcessId = ProcessId;
            Input.BaseAddress = BaseAddress;
            Input.Buffer = reinterpret_cast<WdkTypes::PVOID>(Buffer);
            Input.Size = Size;
            return KbSendRequest(Ctls::KbReadProcessMemory, &Input, sizeof(Input));
        }

        BOOL WINAPI KbWriteProcessMemory(ULONG ProcessId, OUT WdkTypes::PVOID BaseAddress, IN PVOID Buffer, ULONG Size) {
            if (!ProcessId || !BaseAddress || !Buffer || !Size) return FALSE;
            KB_READ_WRITE_PROCESS_MEMORY_IN Input = {};
            Input.ProcessId = ProcessId;
            Input.BaseAddress = BaseAddress;
            Input.Buffer = reinterpret_cast<WdkTypes::PVOID>(Buffer);
            Input.Size = Size;
            return KbSendRequest(Ctls::KbWriteProcessMemory, &Input, sizeof(Input));
        }
    }

    namespace Apc {
        BOOL WINAPI KbQueueUserApc(ULONG ThreadId, _ApcProc ApcProc, PVOID Argument) {
            KB_QUEUE_USER_APC_IN Input = {};
            Input.ThreadId = ThreadId;
            Input.ApcProc = reinterpret_cast<WdkTypes::PVOID>(ApcProc);
            Input.Argument = reinterpret_cast<WdkTypes::PVOID>(Argument);
            return KbSendRequest(Ctls::KbQueueUserApc, &Input, sizeof(Input));
        }
    }
}

namespace KernelShells {
    BOOL WINAPI KbExecuteShellCode(_ShellCode ShellCode, PVOID Argument, OUT OPTIONAL PULONG Result) {
        KB_EXECUTE_SHELL_CODE_IN Input = {};
        KB_EXECUTE_SHELL_CODE_OUT Output = {};
        Input.Address = reinterpret_cast<WdkTypes::PVOID>(ShellCode);
        Input.Argument = reinterpret_cast<WdkTypes::PVOID>(Argument);
        BOOL Status = KbSendRequest(Ctls::KbExecuteShellCode, &Input, sizeof(Input), &Output, sizeof(Output));
        if (Result) *Result = Output.Result;
        return Status;
    }
}

namespace LoadableModules {
    BOOL WINAPI KbCreateDriver(LPCWSTR DriverName, WdkTypes::PVOID DriverEntry)
    {
        KB_CREATE_DRIVER_IN Input = {};
        SIZE_T NameLength = 0;
        __try {
            NameLength = wcslen(DriverName);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        if (NameLength > 64) {
            SetLastError(ERROR_INVALID_NAME);
            return FALSE; // Very long name, seems like invalid data buffer
        }
        Input.DriverEntry = DriverEntry;
        Input.DriverName = reinterpret_cast<WdkTypes::PVOID>(DriverName);
        Input.DriverNameSizeInBytes = static_cast<ULONG>(NameLength) * sizeof(WCHAR); // We're sure that Length <= 64
        return KbSendRequest(Ctls::KbCreateDriver, &Input, sizeof(Input));
    }

    BOOL WINAPI KbLoadModule(
        WdkTypes::HMODULE hModule,
        LPCWSTR ModuleName,
        OPTIONAL WdkTypes::PVOID OnLoad,
        OPTIONAL WdkTypes::PVOID OnUnload,
        OPTIONAL WdkTypes::PVOID OnDeviceControl
    ) {
        if (!hModule || !ModuleName) return FALSE;
        KB_LOAD_MODULE_IN Input = {};
        Input.hModule = hModule;
        Input.ModuleName = reinterpret_cast<WdkTypes::LPCWSTR>(ModuleName);
        Input.OnLoad = OnLoad;
        Input.OnUnload = OnUnload;
        Input.OnDeviceControl = OnDeviceControl;
        return KbSendRequest(Ctls::KbLoadModule, &Input, sizeof(Input));
    }

    BOOL WINAPI KbUnloadModule(WdkTypes::HMODULE hModule)
    {
        if (!hModule) return FALSE;
        KB_UNLOAD_MODULE_IN Input = {};
        Input.hModule = hModule;
        return KbSendRequest(Ctls::KbUnloadModule, &Input, sizeof(Input));
    }

    BOOL WINAPI KbGetModuleHandle(LPCWSTR ModuleName, OUT WdkTypes::HMODULE* hModule)
    {
        if (!ModuleName || !hModule) return FALSE;
        KB_GET_MODULE_HANDLE_IN Input = {};
        KB_GET_MODULE_HANDLE_OUT Output = {};
        Input.ModuleName = reinterpret_cast<WdkTypes::LPCWSTR>(ModuleName);
        BOOL Status = KbSendRequest(Ctls::KbGetModuleHandle, &Input, sizeof(Input), &Output, sizeof(Output));
        *hModule = Output.hModule;
        return Status;
    }

    BOOL WINAPI KbCallModule(WdkTypes::HMODULE hModule, ULONG CtlCode, OPTIONAL WdkTypes::PVOID Argument)
    {
        if (!hModule) return FALSE;
        KB_CALL_MODULE_IN Input = {};
        Input.hModule = hModule;
        Input.CtlCode = CtlCode;
        Input.Argument = Argument;
        return KbSendRequest(Ctls::KbCallModule, &Input, sizeof(Input));
    }
}

namespace Stuff {
    BOOL WINAPI KbGetKernelProcAddress(LPCWSTR RoutineName, WdkTypes::PVOID* KernelAddress)
    {
        if (!RoutineName || !KernelAddress) return FALSE;
        SIZE_T NameLength = 0;
        __try {
            NameLength = wcslen(RoutineName);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        if (NameLength > 64) {
            SetLastError(ERROR_INVALID_NAME);
            return FALSE; // Very long name, seems like invalid data buffer
        }
        KB_GET_KERNEL_PROC_ADDRESS_IN Input = {};
        KB_GET_KERNEL_PROC_ADDRESS_OUT Output = {};
        Input.RoutineName = reinterpret_cast<WdkTypes::LPCWSTR>(RoutineName);
        Input.SizeOfBufferInBytes = static_cast<ULONG>(NameLength) * sizeof(WCHAR); // We're sure that Length <= 64
        BOOL Status = KbSendRequest(Ctls::KbGetKernelProcAddress, &Input, sizeof(Input), &Output, sizeof(Output));
        *KernelAddress = Output.Address;
        return Status;
    }

    BOOL WINAPI KbStallExecutionProcessor(ULONG Microseconds)
    {
        KB_STALL_EXECUTION_PROCESSOR_IN Input = {};
        Input.Microseconds = Microseconds;
        return KbSendRequest(Ctls::KbStallExecutionProcessor, &Input, sizeof(Input));
    }

    BOOL WINAPI KbBugCheck(ULONG Status)
    {
        KB_BUG_CHECK_IN Input = {};
        Input.Status = Status;
        return KbSendRequest(Ctls::KbBugCheck, &Input, sizeof(Input));
    }

    BOOL WINAPI KbFindSignature(
        OPTIONAL ULONG ProcessId,
        WdkTypes::PVOID Memory,
        ULONG Size,
        LPCSTR Signature,
        LPCSTR Mask, 
        OUT WdkTypes::PVOID* FoundAddress
    ) {
        if (!FoundAddress) return FALSE;
        KB_FIND_SIGNATURE_IN Input = {};
        KB_FIND_SIGNATURE_OUT Output = {};
        Input.ProcessId = ProcessId;
        Input.Memory = Memory;
        Input.Size = Size;
        Input.Signature = reinterpret_cast<WdkTypes::LPCSTR>(Signature);
        Input.Mask = reinterpret_cast<WdkTypes::LPCSTR>(Mask);
        BOOL Status = KbSendRequest(Ctls::KbFindSignature, &Input, sizeof(Input), &Output, sizeof(Output));
        *FoundAddress = Output.Address;
        return Status;
    }
}