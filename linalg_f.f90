module linalg
use omp_lib
! Pure Fortran implementation of the matmul routines
implicit none
    
interface
    subroutine matrix_multiply_2d_gpu(h_matrix_a, h_matrix_b, h_matrix_c, N, M, K) &
                                 bind(c, name="matrix_multiply_2d_gpu")
        use iso_c_binding
        real, dimension(:), intent(in) :: h_matrix_a, h_matrix_b
        real, dimension(:), intent(out) :: h_matrix_c
        integer, intent(in) :: N, M, K
    end subroutine matrix_multiply_2d_gpu
end interface

integer, parameter :: sp = kind(0.0)

contains

    subroutine matmul_2d(A, B, C)
        implicit none
        real(sp), intent(in) :: A(:,:), B(:,:)
        real(sp), intent(out) :: C(:,:) ! real(sp), intent(out) :: C(size(A,1), size(B,2))
        integer :: ii, jj, kk
        integer :: n, m, k

        n = size(A, 1)
        k = size(A, 2)
        m = size(B, 2)
        ! write(*,*) n,m,p
        C = 0.0_sp
        
        do ii = 1, n
         do jj = 1, m
            do kk = 1, k
                C(ii, jj) = C(ii, jj) + A(ii, kk) * B(kk, jj)
            end do
         end do
        end do
        
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
        !write(*,*) n,m,p
        do i = 1, n
          do j = 1, p
            do k = 1, m
                C(i, j) = C(i, j) + A(k, i) * B(k, j)
            end do
         end do
        end do

    end subroutine matmul_2d_t

end module
