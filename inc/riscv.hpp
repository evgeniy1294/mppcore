/**
  ***********************************************************
  @author Evgenii Fedoseev
  @file   /src/core/riscv.hpp
  @brief  Compatible with riscv core
  ***********************************************************
**/

#pragma once

//____________________INCLUDE_____________________//
#include <type_traits>


namespace mpp::core {
  
  enum class Accessibility { MRW, MRO, URW, URO };
  
  
  
  
  // TODO: Change std::uint32_t to xlen_t
  // TODO: Change __builtin_constant_p to std::is_constant_evaluated() (C++20)
  template< std::uintptr_t tAddress, Accessibility tAccessibility >
  class AbstractCsr
  {
    public:
  
      static constexpr std::uintptr_t kAddress = tAddress;
      static constexpr Accessibility kAccessibility = tAccessibility;
    
      static_assert ( kAddress < 4096, "Wrong CSR address" );
    
      inline static void Write [[gnu::always_inline]] (std::uint32_t value) {
        static_assert(kAccessibility == Accessibility::MRW || kAccessibility == Accessibility::URW, "This CSR read only");
        
        if ( __builtin_constant_p(value) && value < 32u ) {
          asm volatile ("csrrwi  x0, %[csr], %[uimm]" 
                        : /* No output parameters */
                        : [csr] "i"(kAddress), [uimm] "i"(value) // <- Input parameters
                        : /* No clobbers */ );  
        }
        else {
          asm volatile ("csrw  %[csr], %[aval]" 
                        : /* No output parameters */
                        : [csr] "i"(kAddress), [aval] "r"(value) // <- Input parameters
                        : /* No clobbers */ );  
        }
      }
    
      inline static std::uint32_t Read [[gnu::always_inline]] () {
        std::uint32_t ret;
        asm volatile ("csrr %[dest], %[csr]" 
                      : [dest] "=r"(ret)      // <- Output parameters
                      : [csr]  "i"(kAddress)  // <- Input parameters
                      : /* No clobbers */ );
    
        return ret;
      }
    
      inline static void Set [[gnu::always_inline]] (std::uint32_t mask) {
        static_assert(kAccessibility == Accessibility::MRW || kAccessibility == Accessibility::URW, "This CSR read only");
        
        if ( __builtin_constant_p(mask) && mask < 32u) {
          asm volatile ("csrsi  %[csr], %[uimm]" 
                        : /* No output parameters */
                        : [csr] "i"(kAddress), [uimm] "i"(mask) // <- Input parameters
                        : /* No clobbers */ );  
        }
        else {
          asm volatile ("csrs  %[csr], %[aval]" 
                        : /* No output parameters */
                        : [csr] "i"(kAddress), [aval] "r"(mask) // <- Input parameters
                        : /* No clobbers */ );  
        }
      }
    
      inline static void Clear [[gnu::always_inline]] (std::uint32_t mask) {
        static_assert(kAccessibility == Accessibility::MRW || kAccessibility == Accessibility::URW, "This CSR read only");
        
        if ( __builtin_constant_p(mask) && mask < 32u) {
          asm volatile ("csrci  %[csr], %[uimm]" 
                        : /* No output parameters */
                        : [csr] "i"(kAddress), [uimm] "i"(mask) // <- Input parameters
                        : /* No clobbers */ );  
        }
        else {
          asm volatile ("csrc  %[csr], %[aval]" 
                        : /* No output parameters */
                        : [csr] "i"(kAddress), [aval] "r"(mask) // <- Input parameters
                        : /* No clobbers */ );  
        }
      }
  }; 
  
  
    // RISC-V Standart CSR (Machine mode)
    using MVENDORID         = AbstractCsr < 0xF11ul, Accessibility::MRO >;  //!< Machine Vendor ID Register
    using MARCHID           = AbstractCsr < 0xF12ul, Accessibility::MRO >;  //!< Machine Architecture Register
    using MIMPID            = AbstractCsr < 0xF13ul, Accessibility::MRO >;  //!< Machine Implementation ID Register
    using MHARTID           = AbstractCsr < 0xF14ul, Accessibility::MRO >;  //!< Machine Hart ID Register
    using MSTATUS           = AbstractCsr < 0x300ul, Accessibility::MRW >;  //!< Machine Status Register
    using MISA              = AbstractCsr < 0x301ul, Accessibility::MRO >;  //!< Machine ISA Register
    using MIE               = AbstractCsr < 0x304ul, Accessibility::MRW >;  //!< Machine Interrupt Enable Register
    using MTVEC             = AbstractCsr < 0x305ul, Accessibility::MRW >;  //!< Machine Trap-Vector Base-Address Register
    using MTVT              = AbstractCsr < 0x307ul, Accessibility::MRW >;  //!< Machine ECLIC Interrupt Vector Table Base Address Register
    using MSCRATCH          = AbstractCsr < 0x340ul, Accessibility::MRW >;  //!< Machine Scratch Register
    using MEPC              = AbstractCsr < 0x341ul, Accessibility::MRW >;  //!< Machine Exception Program Counter Register
    using MCAUSE            = AbstractCsr < 0x342ul, Accessibility::MRW >;  //!< Machine Cause Register
    using MTVAL             = AbstractCsr < 0x343ul, Accessibility::MRW >;  //!< Machine Trap Value Register
    using MIP               = AbstractCsr < 0x344ul, Accessibility::MRW >;  //!< Machine Interrupt Pending Register
    using MNXTI             = AbstractCsr < 0x345ul, Accessibility::MRW >;  //!< Machine the Next Interrupt handler address and enable modifier
    using MINTSTATUS        = AbstractCsr < 0x346ul, Accessibility::MRO >;  //!< Machine Current Interrupt Level Register
    using MSCRATCHCSW       = AbstractCsr < 0x348ul, Accessibility::MRW >;  //!< Machine Scratch Swap Register for privileged mode
    using MSCRATCHCSWL      = AbstractCsr < 0x349ul, Accessibility::MRW >;  //!< Machine Scratch Swap Register for interrupt levels
    using MCYCLE            = AbstractCsr < 0xB00ul, Accessibility::MRW >;  //!< Machine Lower 32-bits of Cycle counter 
    using MCYCLEH           = AbstractCsr < 0xB80ul, Accessibility::MRW >;  //!< Machine Upper 32-bits of Cycle counter 
    using MINSTRET          = AbstractCsr < 0xB02ul, Accessibility::MRW >;  //!< Machine Lower 32-bits of Instruction-retired counter
    using MINSTRETH         = AbstractCsr < 0xB82ul, Accessibility::MRW >;  //!< Machine Upper 32-bits of Instruction-retired counter
    
    
    // RISC-V Standart CSR (User Mode)
    using CYCLE             = AbstractCsr < 0xC00ul, Accessibility::URO >;  //!< MCYCLE read-only copy
    using TIME              = AbstractCsr < 0xC01ul, Accessibility::URO >;  //!< MTIME read-only copy
    using ISTRET            = AbstractCsr < 0xC02ul, Accessibility::URO >;  //!< MINSTRET read-only copy
    using CYCLEH            = AbstractCsr < 0xC80ul, Accessibility::URO >;  //!< MCYCLEH read-only copy
    using TIMEH             = AbstractCsr < 0xC81ul, Accessibility::URO >;  //!< MTIMEH read-only copy
    using ISTRETH           = AbstractCsr < 0xC82ul, Accessibility::URO >;  //!< MINSTRETH read-only copy
  
} // namespace mpp::core
