!
! Distributed under the OSI-approved Apache License, Version 2.0.  See
!  accompanying file Copyright.txt for details.
!
!  adios2_adios_init_mod_mpi.F90 : ADIOS2 Fortran bindings for ADIOS
!                                  class Init functions (MPI variants)
!

#ifdef ADIOS2_HAVE_FORTRAN_SUBMODULES
# define ADIOS2_MODULE_PROCEDURE module
#else
# define ADIOS2_MODULE_PROCEDURE
#endif

#ifdef ADIOS2_HAVE_FORTRAN_SUBMODULES
submodule ( adios2_adios_init_mod ) adios2_adios_init_mpi_smod
#else
module adios2_adios_init_mpi_mod
#endif

    use adios2_parameters_mod
    use adios2_functions_mod
    implicit none

#ifndef ADIOS2_HAVE_FORTRAN_SUBMODULES
    interface adios2_init
        module procedure adios2_init_mpi
        module procedure adios2_init_debug_mpi
        module procedure adios2_init_config_mpi
        module procedure adios2_init_config_debug_mpi
    end interface
#endif
    external adios2_init_config_mpi_f2c

contains

    ADIOS2_MODULE_PROCEDURE subroutine adios2_init_mpi( &
            adios, comm, adios2_debug_mode, ierr)
        type(adios2_adios), intent(out) :: adios
        integer, intent(in) :: comm
        logical, intent(in) :: adios2_debug_mode
        integer, intent(out) :: ierr

        call adios2_init_config_mpi(adios, char(0), comm, adios2_debug_mode, ierr)

    end subroutine

    ADIOS2_MODULE_PROCEDURE subroutine adios2_init_debug_mpi(adios, comm, ierr)
        type(adios2_adios), intent(out) :: adios
        integer, intent(in) :: comm
        integer, intent(out) :: ierr

        call adios2_init_config_mpi(adios, char(0), comm, .true., ierr)

    end subroutine

    ADIOS2_MODULE_PROCEDURE subroutine adios2_init_config_mpi( &
            adios, config_file, comm, adios2_debug_mode, ierr)
        type(adios2_adios), intent(out) :: adios
        character*(*), intent(in) :: config_file
        integer, intent(in) :: comm
        logical, intent(in) :: adios2_debug_mode
        integer, intent(out) :: ierr
        ! local
        integer debug_mode

        debug_mode = adios2_LogicalToInt(adios2_debug_mode)
        call adios2_init_config_mpi_f2c(adios%f2c, &
                                        TRIM(ADJUSTL(config_file))//char(0), &
                                        comm, debug_mode, ierr)
        if( ierr == 0 ) adios%valid = .true.

    end subroutine

    ADIOS2_MODULE_PROCEDURE subroutine adios2_init_config_debug_mpi( &
            adios, config_file, comm, ierr)
        type(adios2_adios), intent(out) :: adios
        character*(*), intent(in) :: config_file
        integer, intent(in) :: comm
        integer, intent(out) :: ierr

        call adios2_init_config_mpi(adios, config_file, comm, .true., ierr)

    end subroutine

#ifdef ADIOS2_HAVE_FORTRAN_SUBMODULES
end submodule
#else
end module
#endif
