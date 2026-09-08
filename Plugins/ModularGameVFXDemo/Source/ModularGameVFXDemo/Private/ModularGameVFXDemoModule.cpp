#include "Modules/ModuleManager.h"
#if WITH_EDITOR
#include "Editor.h"
#include "Framework/Commands/InputBindingManager.h"
#include "Framework/Commands/UICommandInfo.h"
#endif

class FModularGameVFXDemoModule : public FDefaultGameModuleImpl {
public:
 virtual void StartupModule() override {
#if WITH_EDITOR
  FEditorDelegates::BeginPIE.AddRaw(this,&FModularGameVFXDemoModule::BeginReview);
  FEditorDelegates::EndPIE.AddRaw(this,&FModularGameVFXDemoModule::EndReview);
#endif
 }
 virtual void ShutdownModule() override {
#if WITH_EDITOR
  EndReview(false);FEditorDelegates::BeginPIE.RemoveAll(this);FEditorDelegates::EndPIE.RemoveAll(this);
#endif
 }
#if WITH_EDITOR
private:
 TSharedPtr<FUICommandInfo> StopCommand;
 FInputChord PreviousStop;
 void BeginReview(bool) {
  StopCommand=FInputBindingManager::Get().FindCommandInContext(TEXT("PlayWorld"),FName(TEXT("StopPlaySession")));
  if(StopCommand.IsValid()){
   PreviousStop=*StopCommand->GetActiveChord(EMultipleKeyBindingIndex::Primary);
   if(PreviousStop==FInputChord(EKeys::Escape)){
    StopCommand->SetActiveChord(FInputChord(EKeys::Escape,true,false,false,false),EMultipleKeyBindingIndex::Primary);
    UE_LOG(LogTemp,Display,TEXT("FM_REVIEW_STOP_SHIFT_ESCAPE"));
   }else StopCommand.Reset();
  }
 }
 void EndReview(bool) {
  if(StopCommand.IsValid()){StopCommand->SetActiveChord(PreviousStop,EMultipleKeyBindingIndex::Primary);StopCommand.Reset();}
 }
#endif
};
IMPLEMENT_MODULE(FModularGameVFXDemoModule, ModularGameVFXDemo);
