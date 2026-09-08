from pathlib import Path
p=Path('F:/game/MythicVFXLab/Source/MythicVFXLab/FrostMageGameplay.cpp')
s=p.read_text()
start=s.index(' const float X=W/2-431,Y=H-103;')
end=s.index(' DrawRect(FLinearColor(.006f,.012f,.024f,.85f),20,99',start)
s=s[:start]+''' const int Cols=W<1150?4:7,Rows=Cols==4?2:1;
 const float CardW=FMath::Min(170.f,(W-64-(Cols-1)*8)/Cols),Gap=8,BarW=Cols*CardW+(Cols-1)*Gap;
 const float X=(W-BarW)/2,Y=H-102-(Rows-1)*72;
 DrawRect(FLinearColor(.004f,.009f,.022f,.91f),X-10,Y-10,BarW+20,Rows*72+12);
 const TCHAR* Names[]={TEXT("FROSTBOLT"),TEXT("FROST NOVA"),TEXT("ROSE BLOOM"),TEXT("HEALING BLOOM"),TEXT("FIREBALL"),TEXT("FIRE BLAST"),TEXT("FLAMESTRIKE")};
 const TCHAR* Hints[]={TEXT("24m / Cast 1.5s"),TEXT("Instant / Root 4s"),TEXT("Petals / 6s"),TEXT("+12 HP/s / 6s"),TEXT("30 dmg / 1.8s"),TEXT("22 dmg / Instant"),TEXT("AoE + burn / 2s")};
 const float CDs[]={P->BoltCooldown,P->NovaCooldown,P->PetalCooldown,P->HealingCooldown,P->FireballCooldown,P->FireBlastCooldown,P->FlamestrikeCooldown};
 const float MaxCDs[]={.65f,6.f,8.f,8.f,.5f,8.f,10.f};
 for(int i=0;i<7;i++){
  const float SX=X+(i%Cols)*(CardW+Gap),SY=Y+(i/Cols)*72,CD=CDs[i];
  const FLinearColor Accent=i>=4?FLinearColor(1,.32f,.065f):(i==3?FLinearColor(.13f,.82f,.58f):(i==2?FLinearColor(.98f,.28f,.58f):FLinearColor(.19f,.49f,.72f)));
  DrawRect(i>=4?FLinearColor(.16f,.044f,.018f,.96f):FLinearColor(.03f,.10f,.18f,.96f),SX,SY,CardW,62);DrawRect(Accent,SX,SY,3,62);
  if(CD>0)DrawRect(FLinearColor(.003f,.009f,.025f,.60f),SX,SY,CardW*FMath::Clamp(CD/MaxCDs[i],0.f,1.f),62);
  Label(FString::FromInt(i+1),SX+9,SY+9,1.3f,Accent);Label(Names[i],SX+33,SY+9,.71f);
  Label(CD>0?FString::Printf(TEXT("%.1fs"),CD):Hints[i],SX+33,SY+35,.64f,CD>0?FLinearColor(.9f,.69f,.3f):FLinearColor(.58f,.70f,.82f));
 }
 if(P->CastingSpell){
  const float BX=W/2-185,BY=Y-47;DrawRect(FLinearColor(.006f,.013f,.03f,.9f),BX,BY,370,24);
  const FLinearColor Tint=P->CastingSpell>=5?FLinearColor(.95f,.26f,.025f,.9f):(P->CastingSpell==4?FLinearColor(.06f,.70f,.42f,.9f):FLinearColor(.17f,.56f,.88f,.9f));
  DrawRect(Tint,BX+2,BY+2,366*(1-P->CastRemaining/FMath::Max(.01f,P->CastDuration)),20);
  const TCHAR* CastNames[]={TEXT(""),TEXT("Conjuring Frostbolt..."),TEXT("Unleashing Frost Nova..."),TEXT("Summoning Rose Bloom..."),TEXT("Invoking Healing Bloom..."),TEXT("Conjuring Fireball..."),TEXT("Fire Blast"),TEXT("Calling Flamestrike...")};
  Label(CastNames[FMath::Clamp(P->CastingSpell,0,7)],BX+10,BY+2,.8f);
 }
''' +s[end:]
s=s.replace('TEXT("M O O N F R O S T")','TEXT("E M B E R  &  F R O S T")')
s=s.replace('W/2-220,H-198,.82f','W/2-220,Y-79,.82f')
s=s.replace('Message(TEXT("1 Frostbolt  |  2 Frost Nova  |  3 Rose Bloom  |  4 Healing Bloom"))','Message(TEXT("1-4 Frost / Bloom  |  5 Fireball  |  6 Fire Blast  |  7 Flamestrike"))')
p.write_text(s)
