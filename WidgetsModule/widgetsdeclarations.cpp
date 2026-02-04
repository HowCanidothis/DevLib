#include "widgetsdeclarations.h"

#include <QLabel>

namespace WidgetProperties {
IMPLEMENT_GLOBAL_CHAR_1(ExtraFieldsCount);
IMPLEMENT_GLOBAL_CHAR(InnerSplitter, inner);
IMPLEMENT_GLOBAL_CHAR_1(Footer)
IMPLEMENT_GLOBAL_CHAR_1(InvertedModel)
IMPLEMENT_GLOBAL_CHAR_1(ActionWidget)
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::ReplaceButton()
{
    return SaveRoleButton(TR(tr("Replace")));
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::ImportButton()
{
    return SaveRoleButton(TR(tr("Import")));
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::MergeButton()
{
    return SaveRoleButton(TR(tr("Merge")));
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::InsertButton()
{
    return SaveRoleButton(TR(tr("Insert")));
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::CloseButton()
{
    return SaveRoleButton(TR(tr("Close")));
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::ApplyButton()
{
    return SaveRoleButton(TR(tr("Apply")));
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::SelectButton()
{
    return SaveRoleButton(TR(tr("Select")));
}

WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::SaveButton()
{
    return SaveRoleButton(TR(tr("Save")));
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::ConfirmButton()
{
    return SaveRoleButton(TR(tr("Confirm")));
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::OkButton()
{
    return SaveRoleButton(TR(tr("Ok")));
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::CancelButton()
{
    return CancelRoleButton(TR(tr("Cancel")));
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::DiscardButton()
{
    return DiscardRoleButton(TR(tr("Discard")));
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::DeleteButton()
{
    return DiscardRoleButton(TR(tr("Delete")));
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::SaveRoleButton(const FTranslationHandler& text)
{
    return WidgetsDialogsManagerButtonStruct(QDialogButtonBox::YesRole, text);
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::CancelRoleButton(const FTranslationHandler& text)
{
    return WidgetsDialogsManagerButtonStruct(QDialogButtonBox::NoRole, text);
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::DiscardRoleButton(const FTranslationHandler& text)
{
    return WidgetsDialogsManagerButtonStruct(QDialogButtonBox::DestructiveRole, text);
}
WidgetsDialogsManagerButtonStruct WidgetsDialogsManagerDefaultButtons::ActionRoleButton(const FTranslationHandler& text, const Name& icon)
{
    return WidgetsDialogsManagerButtonStruct(QDialogButtonBox::ActionRole, text, icon);
}

DescCustomDialogParams& DescCustomDialogParams::FillWithText(const QString& text)
{
    Q_ASSERT(View == nullptr);
    View = new QLabel(text);
    View->setContentsMargins(0,0,9,0);
    return *this;
}
