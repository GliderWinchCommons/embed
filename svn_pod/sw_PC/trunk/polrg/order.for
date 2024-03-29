      SUBROUTINE ORDER (M,R,NDEP,K,ISAVE,RX,RY)
	IMPLICIT DOUBLE PRECISION (A-H)
	IMPLICIT DOUBLE PRECISION (O-Z)
      DIMENSION R(1),ISAVE(1),RX(1),RY(1)
C	COPY INTERCORRELATIONS OF INDEPENDENT VARIABLES
C	WITH DEPENDENT VARIABLE
      MM=0
      DO 130 J=1,K
      L2=ISAVE(J)
      IF(NDEP-L2) 122,123,123
  122 L=NDEP+(L2*L2-L2)/2
      GO TO 125
  123 L=L2+(NDEP*NDEP-NDEP)/2
  125 RY(J)=R(L)
C	COPY A SUBSET MATRIX OF INTERCORRELATIONS AMONG
C	INDEPENDENT VARIABLES
      DO 130 I=1,K
      L1=ISAVE(I)
      IF(L1-L2) 127, 128, 128
  127 L=L1+(L2*L2-L2)/2
      GO TO 129
  128 L=L2+(L1*L1-L1)/2
  129 MM=MM+1
  130 RX(MM)=R(L)
C	PLACE THE SUBSCRIPT NUMBER OF THE DEPENDENT
C	VARIABLE IN ISAVE(K+1)
      ISAVE(K+1)=NDEP
      RETURN
      END

