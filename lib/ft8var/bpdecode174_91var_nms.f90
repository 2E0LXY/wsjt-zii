subroutine bpdecode174_91var_nms(llr,apmask,maxiterations,message77,cw,nharderror,iter)
!
! A log-domain belief propagation decoder for the (174,91) code, using the
! Normalized Min-Sum (NMS) check-node update in place of the exact
! sum-product tanh()/product()/atanh() chain used by bpdecode174_91var.
!
! Background: sum-product computes, for each check-to-variable message,
!   y = atanh( product_k tanh(x_k/2) )
! which requires one tanh() per incoming edge plus one atanh() per outgoing
! edge -- expensive transcendental calls in the hottest loop of the decoder.
!
! Min-sum approximates this as
!   y_nms = sign( product_k x_k ) * min_k |x_k|
! which is exact when one |x_k| dominates the others, and is a known-good
! approximation otherwise. The plain (unnormalized) form systematically
! overestimates message magnitude versus sum-product; NORMALIZED min-sum
! corrects this with a scalar factor alpha < 1:
!   y_nms = sign( product_k x_k ) * min_k |x_k| * alpha
!
! alpha=0.75 here matches the value reported by the Decodium FT8/FT4/FT2
! decoder family (iu8lmc/Decodium, GPL-3.0) for this same (174,91) code,
! itself consistent with literature-cited ranges of ~0.75-0.87 for
! similar irregular LDPC codes (Chen & Fossorier, "Near optimum universal
! belief propagation based decoding of low-density parity check codes",
! IEEE Trans. Commun., 2002).
!
! IMPORTANT: x_k below is constructed identically to the argument the
! sum-product decoder feeds to tanh() (x_k = -toc(kk,ichk)) so the sign
! convention matches bpdecode174_91var.f90 exactly by direct construction,
! rather than by a separately re-derived sign formula -- this avoids the
! classic min-sum porting bug of an inverted or parity-dependent sign.
!
! This is an EXPERIMENTAL alternate decode path. It compiles and follows
! the published NMS construction faithfully, but its real-world decode
! performance versus the existing sum-product path (bpdecode174_91var)
! has not been empirically validated against live or recorded weak-signal
! traffic. Treat results as provisional until tested on-air.
!
use iso_c_binding, only: c_loc,c_size_t
use crc
integer, parameter:: N=174, K=91, M=N-K
real, parameter:: alpha=0.75      ! normalization factor, see header
integer*1 cw(N)
integer*1, intent(in) :: apmask(N)
integer*1 decoded(K)
integer*1 message77(77)
integer nrw(M),ncw
integer Nm(7,M)
integer Mn(3,N)  ! 3 checks per bit
integer synd(M)
real tov(3,N)
real toc(7,M)
real zn(N)
real, intent(in) :: llr(N)
real sgn,amin,xk

include "ldpc_174_91_c_reordered_parity.f90"

nclast=0
decoded=0
toc=0
tov=0
! initialize messages to checks
do j=1,M
  do i=1,nrw(j)
    toc(i,j)=llr((Nm(i,j)))
  enddo
enddo

ncnt=0

do iter=0,maxiterations

  do i=1,N
    if( apmask(i) .ne. 1 ) then
      zn(i)=llr(i)+sum(tov(1:ncw,i))
    else
      zn(i)=llr(i)
    endif
    cw(i)=0
    if( zn(i) .gt. 0 ) cw(i)=1
  enddo

  ncheck=0
  do i=1,M
    synd(i)=sum(cw(Nm(1:nrw(i),i)))
    if(mod(synd(i),2).ne.0) ncheck=ncheck+1
  enddo

  if( ncheck .eq. 0 ) then ! we have a codeword - if crc is good, return it
    decoded=cw(1:K)
    call chkcrc14a(decoded,nbadcrc)
    nharderror=count( (2*cw-1)*llr .lt. 0.0 )
    if(nbadcrc.eq.0) then
      message77=decoded(1:77)
      return
    endif
  endif

  if( iter.gt.0 ) then  ! this code block implements an early stopping criterion
    nd=ncheck-nclast
    if( nd .lt. 0 ) then ! # of unsatisfied parity checks decreased
      ncnt=0  ! reset counter
    else
      ncnt=ncnt+1
    endif
    if( ncnt .ge. 5 .and. iter .ge. 10 .and. ncheck .gt. 15) then
      nharderror=-1
      return
    endif
  endif
  nclast=ncheck

! Send messages from bits to check nodes
  do j=1,M
    do i=1,nrw(j)
      ibj=Nm(i,j)
      toc(i,j)=zn(ibj)
      do kk=1,ncw ! subtract off what the bit had received from the check
        if( Mn(kk,ibj) .eq. j ) then
          toc(i,j)=toc(i,j)-tov(kk,ibj)
        endif
      enddo
    enddo
  enddo

! send messages from check nodes to variable nodes (Normalized Min-Sum)
  do j=1,N
    do i=1,ncw
      ichk=Mn(i,j)  ! Mn(:,j) are the checks that include bit j
      sgn=1.0
      amin=1.0e30
      do kk=1,nrw(ichk)
        if( Nm(kk,ichk) .ne. j ) then
          xk=-toc(kk,ichk)               ! identical argument to tanh() above
          if( xk .lt. 0.0 ) sgn=-sgn
          amin=min(amin,abs(xk))
        endif
      enddo
      tov(i,j)=2.0*sgn*amin*alpha        ! NMS approximation of 2*atanh(product(tanh(xk/2)))
    enddo
  enddo

enddo
nharderror=-1
return
end subroutine bpdecode174_91var_nms
