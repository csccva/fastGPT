module linalg
use omp_lib
! Pure Fortran implementation of the matmul routines
implicit none

integer, parameter :: sp = kind(0.0)

contains

    ! subroutine matmul_2d(A, B, C)
    ! real(sp), intent(in) :: A(:,:), B(:,:)
    ! real(sp), intent(out) :: C(:,:)

    ! C = matmul(A, B)
    ! end subroutine
    subroutine matmul_2d(A, B, C)
        implicit none
        real(sp), intent(in) :: A(:,:), B(:,:)
        real(sp), intent(out) :: C(:,:) ! real(sp), intent(out) :: C(size(A,1), size(B,2))
        integer :: i, j, k
        integer :: n, m, p

        n = size(A, 1)
        m = size(A, 2)
        p = size(B, 2)

        C = 0.0_sp
        
        !$omp parallel do private(i, j, k) shared(A, B, C)
        do i = 1, n
         do j = 1, p
            do k = 1, m
                C(i, j) = C(i, j) + A(i, k) * B(k, j)
            end do
         end do
        end do
        !$omp end parallel do
    end subroutine matmul_2d

    subroutine matmul_2d_t(A, B, C)
    implicit none
    real(sp), intent(in) :: A(:,:), B(:,:)
    real(sp), intent(out) :: C(:,:) ! real(sp), intent(out) :: C(size(A,2), size(B,2))
    integer :: i, j, k
    integer :: n, m, p

        n = size(A, 2)
        m = size(A, 1)
        p = size(B, 2)

        C = 0.0_sp

        !$omp parallel do private(i, j, k) shared(A, B, C)
        do i = 1, n
          do j = 1, p
            do k = 1, m
                C(i, j) = C(i, j) + A(k, i) * B(k, j)
            end do
         end do
        end do
        !$omp end parallel do
    end subroutine matmul_2d_t

    ! subroutine matmul_2d_t(A, B, C)
    ! real(sp), intent(in) :: A(:,:), B(:,:)
    ! real(sp), intent(out) :: C(:,:)

    ! C = matmul(transpose(A), B)

    ! end subroutine

end module
