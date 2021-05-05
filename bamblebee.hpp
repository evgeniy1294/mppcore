/**
  ***********************************************************
  @author Evgenii Fedoseev
  @file   /src/core/bamblebee_n200.hpp
  @brief  Compatible with gd32f103 family and based on 
          RISC-V N200 core chip
  ***********************************************************
**/

#pragma once

//____________________INCLUDE_____________________//
#include "riscv.hpp"


#define TIMER_MTIME_LO           (TIMER_MTIME)
#define TIMER_MTIME_HI           (TIMER_MTIME + 4)
#define TIMER_MTIMECMP_LO        (TIMER_MTIMECMP)
#define TIMER_MTIMECMP_HI        (TIMER_MTIMECMP + 4)
#define TIMER_MSTOP              (0xFF8)


namespace mpp::core {
    
  namespace __private { std::uint32_t GetTick(); void IncTick(); void ResetTick(); } // namespace __private
    
  inline namespace bamblebee {
    
    // Bumblebee Customized CSR 
    using MCOUNTINHIBIT    = AbstractCsr < 0x320ul, Accessibility::MRW >;  //!< Customized register for counters on & off
    using MNVEC            = AbstractCsr < 0x7C3ul, Accessibility::MRO >;  //!< NMI Entry Address
    using MSUBM            = AbstractCsr < 0x7C4ul, Accessibility::MRW >;  //!< Customized Register Storing Type of Trap
    using MMISC_CTL        = AbstractCsr < 0x7D0ul, Accessibility::MRW >;  //!< Customized Register holding NMI Handler Entry Address
    using MSAVESTATUS      = AbstractCsr < 0x7D6ul, Accessibility::MRW >;  //!< Customized Register holding the value of mstatus
    using MSAVEEPC1        = AbstractCsr < 0x7D7ul, Accessibility::MRW >;  //!< Customized Register holding the value of mepc for the first-level preempted NMI or Exception.
    using MSAVECAUSE1      = AbstractCsr < 0x7D8ul, Accessibility::MRW >;  //!< Customized Register holding the value of mcause for the first-level preempted NMI or Exception.
    using MSAVEEPC2        = AbstractCsr < 0x7D9ul, Accessibility::MRW >;  //!< Customized Register holding the value of mepc for the second-level preempted NMI or Exception.
    using MSAVECAUSE2      = AbstractCsr < 0x7DAul, Accessibility::MRW >;  //!< Customized Register holding the value of mcause for the second-level preempted NMI or Exception.
    using PUSHMSUBM        = AbstractCsr < 0x7EBul, Accessibility::MRW >;  //!< Push msubm to stack
    using MTVT2            = AbstractCsr < 0x7ECul, Accessibility::MRW >;  //!< ECLIC non-vectored interrupt handler address register
    using JALMNXTI         = AbstractCsr < 0x7EDul, Accessibility::MRW >;  //!< Jumping to next interrupt handler address and interrupt-enable register
    using PUSHMCAUSE       = AbstractCsr < 0x7EEul, Accessibility::MRW >;  //!< Push mcause to stack
    using PUSHMEPC         = AbstractCsr < 0x7EFul, Accessibility::MRW >;  //!< Push mepc to stack
    using SLEEPVALUE       = AbstractCsr < 0x811ul, Accessibility::MRW >;  //!< WFI Sleep Mode Register
    using TXEVT            = AbstractCsr < 0x812ul, Accessibility::MRW >;  //!< Send Event Register
    using WFE              = AbstractCsr < 0x810ul, Accessibility::MRW >;  //!< Wait for Event Control Register
    
  
  
  
    template< class ClockSystem >
    class MachineTimer {
      public:
        constexpr static std::uint32_t TickPerSec = 1000u;
        constexpr static std::uint32_t TimeCmp    = ClockSystem::kMachineTimerClkHz / 1000u;
      
        inline static std::uint32_t GetTick() { return __private::GetTick(); }
        
      
        inline static void Init() { 
          TIMER_REG(TIMER_MSTOP) = 1u;
          TIMER_REG(TIMER_MTIME_LO) = 0;
          TIMER_REG(TIMER_MTIME_HI) = 0;
          TIMER_REG(TIMER_MTIMECMP_LO) = TimeCmp;
          TIMER_REG(TIMER_MTIMECMP_HI) = 0;
          
          __private::ResetTick();
          TIMER_REG(TIMER_MSTOP) = 0u;
        }
      
        
        static void InterruptHandler ()
        { 
          TIMER_REG(TIMER_MSTOP) = 1u;
          TIMER_REG(TIMER_MTIME_LO) = 0;
          TIMER_REG(TIMER_MTIME_HI) = 0;
          
          __private::IncTick();
          TIMER_REG(TIMER_MSTOP) = 0u;
        }         
    };
    
      
      
      
    template< class ClockSystem >
    class MachineTickCounter {
      public:
        constexpr static std::uint32_t TickPerSec = ClockSystem::kSysClkHz;
        
        inline static void Init() { 
          MCOUNTINHIBIT::Clear( 1u << 0 /* CSR_MCOUNTINHIBIT_CY */);
        }
        
        inline static std::uint32_t GetTick() { 
          // auto mcountinhibit = MCOUNTINHIBIT::Read();
          
          // MCOUNTINHIBIT::Set( 1u << 0 /* CSR_MCOUNTINHIBIT_CY */);
          // std::uint32_t tmp = CYCLE::Read(); 
          
          // MCOUNTINHIBIT::Write( mcountinhibit );

          return CYCLE::Read(); 
        }
    };
    
    
    
    
    enum class PriorityLevelGroup { L0P4 = 0, L1P3 = 1, L2P2 = 2, L3P1 = 3, L4P0 = 4 };
    
    struct EclicExampleTrait final {
      constexpr static PriorityLevelGroup kPriorityLevelGroup = PriorityLevelGroup::L3P1;
      constexpr static std::size_t kThresholdLevel  = 0;
    };
    
    
    
    
    template < class Trait >
    class Eclic final {
      static_assert(std::is_same_v< Trait, std::decay_t< decltype(Trait()) > >);
      
      private:
        constexpr static bool IsValidTrait() noexcept(true) {
          static_assert( Trait::kThresholdLevel <= 
              ((1 << static_cast< std::uint8_t >(Trait::kPriorityLevelGroup)) - 1), "Invalid threshold level");
          
          return true;
        }    
      
        template < class T, class... Ts >
        constexpr static bool IsValidInterrupt() { 
          return ( (static_cast<std::uint32_t>(T::kInterruptSource) != static_cast<std::uint32_t>(Ts::kInterruptSource)) && ... );
        }

        template < class T, class... Ts >
        constexpr static bool IsValidInterruptGroup()
        {
          if constexpr (sizeof...(Ts) == 0u)
            return true;
          else
            return IsValidInterrupt<T, Ts...>() && IsValidInterruptGroup<Ts...>();
        }
      
        static_assert(IsValidTrait(), "This trait contains error");
      
      public:
      
        constexpr static PriorityLevelGroup kPriorityLevelGroup = Trait::kPriorityLevelGroup;
        constexpr static std::size_t kThresholdLevel = Trait::kThresholdLevel;
       
        constexpr static std::uint8_t kCfgMask = static_cast<std::uint8_t>(kPriorityLevelGroup) << 1;
        constexpr static std::uint8_t kMthMask = kThresholdLevel;
      
      
        template < class... Interrupt >
        constexpr static void Init() {
          static_assert(IsValidInterruptGroup<Interrupt...>(), "All <kInterruptSource> fields must be unique");
          
          typedef volatile uint32_t vuint32_t;

          //clear cfg register 
          *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_CFG_OFFSET)=0;
        
          //clear minthresh register 
          *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_MTH_OFFSET)=0;
        
          //clear all IP/IE/ATTR/CTRL bits for all interrupt sources
          vuint32_t * ptr;
        
          vuint32_t * base = (vuint32_t*)(ECLIC_ADDR_BASE + ECLIC_INT_IP_OFFSET);
          vuint32_t * upper = (vuint32_t*)(base + ECLIC_NUM_INTERRUPTS*4);
        
          for (ptr = base; ptr < upper; ptr=ptr+4){
            *ptr = 0;
          }        
			
          REG32(ECLIC_ADDR_BASE + ECLIC_CFG_OFFSET) = kCfgMask;
          REG32(ECLIC_ADDR_BASE + ECLIC_MTH_OFFSET) = kMthMask;
      
          ( Interrupt::Init(), ... ); 
        }
      
        constexpr static void EmitSoftwareInterrupt() {
          TIMER_REG(TIMER_MSIP) = 1u; 
        }
      
        template < class... Interrupt >
        constexpr static void EnableInterrupts() { ( Interrupt::Enable(), ... ); }
      
      
        template < class... Interrupt >
        constexpr static void DisableInterrupts() { ( Interrupt::Disable(), ... ); }
    };
    
    
    
    
    
    
    enum class Trigger:    uint8_t { Level = 0b000, Posedge = 0b010, Negedge = 0b110 };
    enum class HandleMode: uint8_t { NonVectored = 0b0, Vectored = 0b1 };
    
    
    
     struct IrqExampleTrait final {
        constexpr static HandleMode   kHandleMode = HandleMode::NonVectored;
        constexpr static Trigger      kTrigger    = Trigger::Level;
        constexpr static std::uint8_t kPriorityLevel = 0b1101;
     };
    
         
    
    template < IRQn_Type tInterruptSource, class Trait >
    class Interrupt final {
      public:
        static_assert(::std::is_same_v< Trait, ::std::decay_t< decltype(Trait()) > >);
        static_assert( tInterruptSource < ECLIC_NUM_INTERRUPTS, "Wrong interrupt source ID");
        
        constexpr static IRQn_Type    kInterruptSource = tInterruptSource;
        constexpr static HandleMode   kHandleMode = Trait::kHandleMode;
        constexpr static Trigger      kTrigger = Trait::kTrigger;
        constexpr static std::uint8_t kPriorityLevel = Trait::kPriorityLevel;
            
        constexpr static std::uint8_t kIntAttrMask = static_cast< std::uint8_t >(kTrigger) |
                                                     static_cast< std::uint8_t >(kHandleMode);
        constexpr static std::uint8_t kIntCfgMask  = kPriorityLevel << (8 - ECLICINTCTLBITS);
                              
        inline static void Init() {
          constexpr std::uintptr_t intcfg  = ECLIC_ADDR_BASE + ECLIC_INT_CTRL_OFFSET + static_cast< std::uintptr_t >(kInterruptSource) * 4;
          constexpr std::uintptr_t intattr = ECLIC_ADDR_BASE + ECLIC_INT_ATTR_OFFSET + static_cast< std::uintptr_t >(kInterruptSource) * 4;
          
          *(volatile uint8_t*)intcfg  = kIntCfgMask;
          *(volatile uint8_t*)intattr = kIntAttrMask;
        }
        
        inline static void Enable() {
          constexpr std::uintptr_t intie = ECLIC_ADDR_BASE + ECLIC_INT_IE_OFFSET + static_cast< std::uintptr_t >(kInterruptSource) * 4;
          *(volatile uint8_t*)intie = 1; 
        }
          
        inline static void Disable() {
          constexpr std::uintptr_t intie = ECLIC_ADDR_BASE + ECLIC_INT_IE_OFFSET + static_cast< std::uintptr_t >(kInterruptSource) * 4;
          *(volatile uint8_t*)intie = 0; 
        }
    };
    
    
    
  } // inline namespace
}


