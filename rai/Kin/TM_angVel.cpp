#include "TM_angVel.h"
#include "TM_default.h"
#include "flag.h"

void angVel_base(const rai::KinematicWorld& K0, rai::KinematicWorld& K1, uint i, arr& y, arr& J){
  rai::Frame *f0 = K0.frames(i);
  rai::Frame *f1 = K1.frames(i);

  arr a,b,y_tmp,Ja,Jb;
  K0.kinematicsQuat(a, Ja, f0);
  K1.kinematicsQuat(b, Jb, f1);
  arr J0, J1;
//  quat_diffVector(y, J0, J1, a, b);
  if(scalarProduct(a,b)<0.){
    b*=-1.;
    Jb*=-1.;
  }
  arr dq = b-a;
  a(0) *=-1.;
  quat_concat(y_tmp, J0, J1, dq, a);
  for(uint i=0;i<J1.d0;i++) J1(i,0) *= -1.;
  y_tmp.remove(0);
  J0.delRows(0);
  J1.delRows(0);

  y_tmp *= 2.;
  J0 *= 2.;
  J1 *= 2.;

  y = y_tmp;

#if 0
  y = dq;
  arr FLIP = eye(4); FLIP(0,0)=-1.;
  a = FLIP*a;
  quat_concat(y, J0, J1, dq, a);
  J1 = J1 * FLIP;
  cout <<dq <<'\n' <<J0 <<J1 <<endl;
//  for(uint i=0;i<J1.d0;i++) J1(i,0) *= -1.;
  if(!!J){
    if(Ktuple.N==3){
      J = catCol(zeros(y.N, Ktuple(-3)->q.N), (J1-J0)*Ja, J0*Jb);
    }else{
      J = catCol((J1-J0)*Ja, J0*Jb);
    }
  }
  return;
#endif

#if 0
  f1 = f1->getUpwardLink();
  if(f1->joint->type==rai::JT_free){
    uint i=f1->joint->qIndex;
    y = Ktuple(-1)->q({i+4, i+6});
    if(!!J){
      J.resize(3, Ktuple(-1)->q.N).setZero();
      for(uint j=0;j<3;j++) J(j,i+4+j) = 1.;
      if(Ktuple.N==3){
        J = catCol(zeros(y.N, Ktuple(-3)->q.N), zeros(y.N, Ktuple(-2)->q.N), J);
      }else{
        J = catCol(zeros(y.N, Ktuple(-2)->q.N), J);
      }
    }
    return;
  }else{
    y = b;
  }
  y.remove(0);
  J0.setId(4);
  J0.delRows(0);
  J1.setZero();
#endif

  checkNan(y);

  if(!!J){
    J = catCol((J1-J0)*Ja, J0*Jb);
    checkNan(J);
  }
}

//===========================================================================

void TM_LinVel::phi(arr& y, arr& J, const WorldL& Ktuple){
  if(order==1){
    rai::Frame *f0 = Ktuple(-2)->frames(i);
    rai::Frame *f1 = Ktuple(-1)->frames(i);

    arr a,b,Ja,Jb;
    Ktuple(-2)->kinematicsPos(a, Ja, f0);
    Ktuple(-1)->kinematicsPos(b, Jb, f1);

    y = b-a;
    if(!!J){
      expandJacobian(Ja, Ktuple, -2);
      expandJacobian(Jb, Ktuple, -1);
      J = Jb-Ja;
    }

#if 1
    if(Ktuple(-1)->hasTimeJoint()){
      double tau; arr Jtau;
      Ktuple(-1)->kinematicsTau(tau, (!!J?Jtau:NoArr));
      CHECK_GE(tau, 1e-10, "");
      y /= tau;
      if(!!J){
        J /= tau;
        expandJacobian(Jtau, Ktuple, -1);
        J += (-1./tau)*y*Jtau;
      }
    }else{
      double tau = Ktuple(-1)->frames(0)->tau;
      CHECK_GE(tau, 1e-10, "");
      y /= tau;
      if(!!J) J /= tau;
    }
#endif
    return;
  }

  if(order==2){
    arr y0, y1, Jy0, Jy1;
    order--;
    phi(y0, (!!J?Jy0:NoArr), Ktuple({0,-2}));  if(!!J) padJacobian(Jy0, Ktuple);
    phi(y1, (!!J?Jy1:NoArr), Ktuple);
    order++;

    double tau = Ktuple(-2)->frames(0)->tau;
    if(impulseInsteadOfAcceleration) tau=1.;
    y = (y1-y0)/tau; //difference!
    if(!!J) J = (Jy1 - Jy0)/tau;
  }
}

//===========================================================================

void TM_AngVel::phi(arr& y, arr& J, const WorldL& Ktuple) {
  if(order==1){
    arr J_tmp;
    angVel_base(*Ktuple(-2), *Ktuple(-1), i, y, J_tmp);

    if(!!J){
      if(Ktuple.N==3) J = catCol(zeros(y.N, Ktuple(-3)->q.N), J_tmp);
      else J=J_tmp;
    }

#if 1
    if(Ktuple(-1)->hasTimeJoint()){
      double tau; arr Jtau;
      Ktuple(-1)->kinematicsTau(tau, (!!J?Jtau:NoArr));
      CHECK_GE(tau, 1e-10, "");

      y /= tau;
      if(!!J){
        J /= tau;
        expandJacobian(Jtau, Ktuple, -1);
        J += (-1./tau)*y*Jtau;
      }
    }else{
      double tau = Ktuple(-1)->frames(0)->tau;
      CHECK_GE(tau, 1e-10, "");
      y /= tau;
      if(!!J) J /= tau;
    }
#endif
    return;
  }

#if 1
  if(order==2){
    arr y0, y1, Jy0, Jy1;
    order--;
    phi(y0, (!!J?Jy0:NoArr), Ktuple({0,-2}));  if(!!J) padJacobian(Jy0, Ktuple);
    phi(y1, (!!J?Jy1:NoArr), Ktuple);
    order++;

    double tau = Ktuple(-2)->frames(0)->tau;
    if(impulseInsteadOfAcceleration) tau=1.;
    y = (y1-y0)/tau; //difference!
    if(!!J) J = (Jy1 - Jy0)/tau;
  }

#else

  if(order==2){
    arr y0, y1, J0, J1;
    angVel_base(*Ktuple(-3), *Ktuple(-2), i, y0, J0);
    angVel_base(*Ktuple(-2), *Ktuple(-1), i, y1, J1);

    y = y1 - y0;

#if 1
    double tau = Ktuple(-1)->frames(0)->tau;
    CHECK_GE(tau, 1e-10, "");
    double tau2=tau*tau;
    y /= tau2;
#endif

    if(!!J){
      CHECK_EQ(Ktuple.N, 3,"");
      uint d0=Ktuple(-3)->q.N;
      uint d1=Ktuple(-2)->q.N;
      uint d2=Ktuple(-1)->q.N;
      J.resize(y.N, d0+d1+d2).setZero();
      CHECK_EQ(J0.d1, d0+d1, "");
      CHECK_EQ(J1.d1, d1+d2, "");
      for(uint i=0;i<y.N;i++){
        for(uint j=0;j<J0.d1;j++) J(i,j) -= J0(i,j);
        for(uint j=0;j<J1.d1;j++) J(i,d0+j) += J1(i,j);
      }

#if 1
      J /= tau2;
      arr Jtau;  Ktuple(-1)->jacobianTime(Jtau, Ktuple(-1)->frames(0));  expandJacobian(Jtau, Ktuple, -1);
      J += (-2./tau)*y*Jtau;
#endif
    }
    return;
  }

  HALT("shoudn't be here");
#endif

}

uint TM_AngVel::dim_phi(const rai::KinematicWorld &G){ return 3; }

//===========================================================================

void TM_LinAngVel::phi(arr& y, arr& J, const WorldL& Ktuple){
  y.resize(6);
  if(!!J) J.resize(6, getKtupleDim(Ktuple).last()).setZero();

  if(Ktuple.elem(-2)->frames(i)->flags & (1<<FL_impulseExchange)){
    return;
  }

//  rai::Frame *b0 = Ktuple.elem(-2)->frames(i);    CHECK(&b0->K==Ktuple.elem(-2),"");
//  rai::Frame *b1 = Ktuple.elem(-1)->frames(i);    CHECK(&b1->K==Ktuple.elem(-1),"");
//  cout <<"SWITCH " <<b0->parent->name <<'-' <<b0->name <<" => " <<b1->parent->name <<'-' <<b1->name <<endl;

  TM_LinVel lin(i);
  lin.order=order;
  lin.phi(y({0,2})(), (!!J?J({0,2})():NoArr), Ktuple);

  TM_AngVel ang(i);
  ang.order=order;
  ang.phi(y({3,5})(), (!!J?J({3,5})():NoArr), Ktuple);
}

uint TM_LinAngVel::dim_phi(const rai::KinematicWorld& G){ return 6; }

//===========================================================================

void TM_NoJumpFromParent::phi(arr& y, arr& J, const WorldL& Ktuple){
  rai::Frame *obj = Ktuple.elem(-2)->frames(i);
  rai::Frame *link = obj->getUpwardLink();
  rai::Frame *parent = link->parent;

  if(parent->ID == Ktuple.elem(-1)->frames(i)->getUpwardLink()->parent->ID){
    LOG(-1) <<"this frame isn't switching - are you sure you want to do this?";
  }

  {
//  if(link->joint && link->joint->type==rai::JT_rigid){
    arr yq, Jq;
    TM_Default tmp(TMT_pos, link->ID, NoVector, parent->ID, NoVector);
    tmp.order = 1;
    tmp.type = TMT_pos;
    tmp.Feature::phi(y, J, Ktuple);
    tmp.type = TMT_quat;
    tmp.flipTargetSignOnNegScalarProduct=true;
    tmp.Feature::phi(yq, (!!J?Jq:NoArr), Ktuple);
    y.append(yq);
    if(!!J) J.append(Jq);
  }
//  else{
//    y.resize(7).setZero();
//    if(!!J) J.resize(7,getKtupleDim(Ktuple).last()).setZero();
//  }
}

uint TM_NoJumpFromParent::dim_phi(const rai::KinematicWorld& G){
  return 7;
}
