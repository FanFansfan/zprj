//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//*****************************************************************//
// $Archive: /Alaska/BIN/Modules/Template/FixedBootOrder Demo/Board/em/FixedBootOrder/FixedBootOrderHII.c $
//
// $Author: Easonchen $
//
// $Revision: 4 $
//
// $Date: 8/12/13 4:20a $
//
//*****************************************************************//
//*****************************************************************//
// Revision History
// ----------------
// $Log: /Alaska/BIN/Modules/Template/FixedBootOrder Demo/Board/em/FixedBootOrder/FixedBootOrderHII.c $
// 
// 4     8/12/13 4:20a Easonchen
// [TAG]  		N/A
// [Category]  	Improvement
// [Description]  	CppCheck error fix.
// [Files]  		FixedBootOrderHII.c
// 
// 3     11/09/12 2:40a Easonchen
// [TAG]  		N/A
// [Category]  	Improvement
// [Description]  	Enhance function CreateHiiOneOfOptionItem()
// [Files]  		FixedBootOrderHii.c
// 
// 2     6/28/12 11:33a Easonchen
// Make a new FixedBootOrder module by utilizing 4.6.4.1 BDS functionality
// 
// 1     6/28/12 11:23a Easonchen
//
//*****************************************************************//


//<AMI_FHDR_START>
//**********************************************************************
//
// Name:        FixedBootOrderHII.c
//
// Description: HiiDatabase Related Functions
//
//**********************************************************************
//<AMI_FHDR_END>

#include <efi.h>
#include <AmiDxeLib.h>
#include <token.h>

#if EFI_SPECIFICATION_VERSION>0x20000
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiString.h>
#include <Protocol/HiiDatabase.h>
#else
#include <Protocol/FormCallback.h>
#include <HIIDB.h>
#include <Protocol/ExtHii.h>
#endif

#include <Setup.h>
#include <SetupStrTokens.h>
#include <AUTOID.h>

#include "FixedBootOrderHII.h"

UINTN FormPackageLength=10;
VOID *hiiFormPackage=NULL;

#if EFI_SPECIFICATION_VERSION>0x20000
#pragma pack(1)

typedef struct _EFI_IFR_GUID_LABEL {
  EFI_IFR_OP_HEADER   Header;
  EFI_GUID            Guid;
  UINT8               ExtendOpCode;
  UINT16              Number;
} EFI_IFR_GUID_LABEL;
#pragma pack()
//
// GUIDed opcodes defined for Tiano
//
#define EFI_IFR_TIANO_GUID \
  { 0xf0b1735, 0x87a0, 0x4193, 0xb2, 0x66, 0x53, 0x8c, 0x38, 0xaf, 0x48, 0xce }

EFI_GUID mIfrVendorGuid = EFI_IFR_TIANO_GUID;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EfiCompareGuid
//
// Description:	To avoid including EDK libs.
//
// Input:		IN EFI_GUID *Guid1,  IN EFI_GUID *Guid2
//
// Output:		BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
EfiCompareGuid (
  IN EFI_GUID *Guid1,
  IN EFI_GUID *Guid2
  )
{
	return (BOOLEAN)(0==MemCmp((UINT8*)Guid1,(UINT8*)Guid2,sizeof(EFI_GUID)));
}

EFI_STATUS
GetPackageDataFromPackageList (
  IN  EFI_HII_PACKAGE_LIST_HEADER *HiiPackageList,
  IN  UINT32                      PackageIndex,
  OUT UINT32                      *BufferLen,
  OUT EFI_HII_PACKAGE_HEADER      **Buffer
  )
{
  UINT32                        Index;
  EFI_HII_PACKAGE_HEADER        *Package;
  UINT32                        Offset;
  UINT32                        PackageListLength;
  EFI_HII_PACKAGE_HEADER        PackageHeader = {0, 0};

  ASSERT(HiiPackageList != NULL);

  if ((BufferLen == NULL) || (Buffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Package = NULL;
  Index   = 0;
  Offset  = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  pBS->CopyMem (&PackageListLength, &HiiPackageList->PackageLength, sizeof (UINT32));
  while (Offset < PackageListLength) {
    Package = (EFI_HII_PACKAGE_HEADER *) (((UINT8 *) HiiPackageList) + Offset);
    pBS->CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));
    if (Index == PackageIndex) {
      break;
    }
    Offset += PackageHeader.Length;
    Index++;
  }
  if (Offset >= PackageListLength) {
    //
    // no package found in this Package List
    //
    return EFI_NOT_FOUND;
  }

  *BufferLen = PackageHeader.Length;
  *Buffer    = Package;
  return EFI_SUCCESS;
}

EFI_STATUS
UpdateFormPackageData (
  IN  EFI_GUID               *FormSetGuid,
  IN  EFI_FORM_ID            FormId,
  IN  EFI_HII_PACKAGE_HEADER *Package,
  IN  UINT32                 PackageLength,
  IN  UINT16                 Label,
  IN  BOOLEAN                Insert,
  IN  EFI_HII_UPDATE_DATA    *Data,
  OUT UINT8                  **TempBuffer,
  OUT UINT32                 *TempBufferSize
  )
{
  UINT8                     *BufferPos;
  EFI_HII_PACKAGE_HEADER    PackageHeader;
  UINT32                    Offset;
  EFI_IFR_OP_HEADER         *IfrOpHdr;
  BOOLEAN                   GetFormSet;
  BOOLEAN                   GetForm;
  UINT8                     ExtendOpCode;
  UINT16                    LabelNumber;
  BOOLEAN                   Updated;

  if ((TempBuffer == NULL) || (TempBufferSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *TempBufferSize = PackageLength;
  if (Data != NULL) {
    *TempBufferSize += Data->Offset;
  }
  *TempBuffer = Malloc (*TempBufferSize);
  if (*TempBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  pBS->CopyMem (*TempBuffer, Package, sizeof (EFI_HII_PACKAGE_HEADER));
  *TempBufferSize = sizeof (EFI_HII_PACKAGE_HEADER);
  BufferPos = *TempBuffer + sizeof (EFI_HII_PACKAGE_HEADER);

  pBS->CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));
  IfrOpHdr   = (EFI_IFR_OP_HEADER *)((UINT8 *) Package + sizeof (EFI_HII_PACKAGE_HEADER));
  Offset     = sizeof (EFI_HII_PACKAGE_HEADER);
  GetFormSet = (FormSetGuid == NULL) ? TRUE : FALSE;
  GetForm    = FALSE;
  Updated    = FALSE;

  while (!Updated && Offset < PackageHeader.Length) {
    pBS->CopyMem (BufferPos, IfrOpHdr, IfrOpHdr->Length);
    BufferPos += IfrOpHdr->Length;
    *TempBufferSize += IfrOpHdr->Length;

    switch (IfrOpHdr->OpCode) {
    case EFI_IFR_FORM_SET_OP :
      if (FormSetGuid != NULL) {
        if (MemCmp (&((EFI_IFR_FORM_SET *) IfrOpHdr)->Guid, FormSetGuid, sizeof (EFI_GUID)) == 0) {
          GetFormSet = TRUE;
        } else {
          GetFormSet = FALSE;
        }
      }
      break;

    case EFI_IFR_FORM_OP:
      if (MemCmp (&((EFI_IFR_FORM *) IfrOpHdr)->FormId, &FormId, sizeof (EFI_FORM_ID)) == 0) {
        GetForm = TRUE;
      } else {
        GetForm = FALSE;
      }
      break;

    case EFI_IFR_GUID_OP :
      if (!GetFormSet || !GetForm) {
        //
        // Go to the next Op-Code
        //
        break;
      }
TRACE((-1,"EFI_IFR_GUID_OP 1\n"));
//	TraceOutData( (UINT8*) &((EFI_IFR_GUID *) IfrOpHdr)->Guid, sizeof(EFI_GUID) );
      if (!EfiCompareGuid (&((EFI_IFR_GUID *) IfrOpHdr)->Guid, &mIfrVendorGuid)) {
        //
        // GUID mismatch, skip this op-code
        //
        break;
      }


      ExtendOpCode = ((EFI_IFR_GUID_LABEL *) IfrOpHdr)->ExtendOpCode;
      pBS->CopyMem (&LabelNumber, &((EFI_IFR_GUID_LABEL *)IfrOpHdr)->Number, sizeof (UINT16));
	
TRACE((-1,"EFI_IFR_GUID_OP 2 ExtendOpCode=%x LabelNumber=%x Label=%x\n",ExtendOpCode, LabelNumber, Label));

      if ((ExtendOpCode != EFI_IFR_EXTEND_OP_LABEL) || (LabelNumber != Label)) {
        //
        // Go to the next Op-Code
        //
        break;
      }

      if (Insert) {
        //
        // Insert data after current Label, skip myself
        //
TRACE((-1,"EFI_IFR_GUID_OP 3\n"));
        Offset   += IfrOpHdr->Length;
        IfrOpHdr = (EFI_IFR_OP_HEADER *) ((CHAR8 *) (IfrOpHdr) + IfrOpHdr->Length);
      } else {
        //
        // Replace data between two paired Label, try to find the next Label.
        //
        while (TRUE) {
          Offset   += IfrOpHdr->Length;
          //
          // Search the next label and Fail if not label found.
          //
          if (Offset >= PackageHeader.Length) {
            goto Fail;
          }
          IfrOpHdr = (EFI_IFR_OP_HEADER *) ((CHAR8 *) (IfrOpHdr) + IfrOpHdr->Length);
          if (IfrOpHdr->OpCode == EFI_IFR_GUID_OP) {
            ExtendOpCode = ((EFI_IFR_GUID_LABEL *) IfrOpHdr)->ExtendOpCode;
            if (EfiCompareGuid (&((EFI_IFR_GUID *) IfrOpHdr)->Guid, &mIfrVendorGuid) && ExtendOpCode == EFI_IFR_EXTEND_OP_LABEL) {
              break;
            }
          }
        }
      }

      //
      // Fill in the update data
      //
      if (Data != NULL) {
TRACE((-1,"EFI_IFR_GUID_OP 4\n"));
        pBS->CopyMem (BufferPos, Data->Data, Data->Offset);
        BufferPos += Data->Offset;
        *TempBufferSize += Data->Offset;
      }

      //
      // Copy the reset data
      //
      pBS->CopyMem (BufferPos, IfrOpHdr, PackageHeader.Length - Offset);
      *TempBufferSize += PackageHeader.Length - Offset;

      Updated = TRUE;
      break;
    default :
      break;
    }

    //
    // Go to the next Op-Code
    //
    Offset   += IfrOpHdr->Length;
    IfrOpHdr = (EFI_IFR_OP_HEADER *) ((CHAR8 *) (IfrOpHdr) + IfrOpHdr->Length);
  }

  //
  // Update the package length.
  //
  PackageHeader.Length = *TempBufferSize;
  pBS->CopyMem (*TempBuffer, &PackageHeader, sizeof (EFI_HII_PACKAGE_HEADER));

Fail:
  if (!Updated) {
    pBS->FreePool (*TempBuffer);
    *TempBufferSize = 0;
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}




EFI_STATUS
IfrLibUpdateForm (
  IN EFI_HII_HANDLE            Handle,
  IN EFI_GUID                  *FormSetGuid, OPTIONAL
  IN EFI_FORM_ID               FormId,
  IN UINT16                    Label,
  IN BOOLEAN                   Insert,
  IN EFI_HII_UPDATE_DATA       *Data
  )
/*++

Routine Description:
  This function allows the caller to update a form that has
  previously been registered with the EFI HII database.

Arguments:
  Handle       - Hii Handle
  FormSetGuid  - The formset should be updated.
  FormId       - The form should be updated.
  Label        - Update information starting immediately after this label in the IFR
  Insert       - If TRUE and Data is not NULL, insert data after Label.
                 If FALSE, replace opcodes between two labels with Data
  Data         - The adding data; If NULL, remove opcodes between two Label.

Returns:
  EFI_SUCCESS  - Update success.
  Other        - Update fail.

--*/
{
  EFI_STATUS                   Status;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINT32                       Index;
  EFI_HII_PACKAGE_LIST_HEADER  *UpdateBuffer;
  UINTN                        BufferSize;
  UINT8                        *UpdateBufferPos;
  EFI_HII_PACKAGE_HEADER       PackageHeader;
  EFI_HII_PACKAGE_HEADER       *Package;
  UINT32                       PackageLength;
  EFI_HII_PACKAGE_HEADER       *TempBuffer;
  UINT32                       TempBufferSize;
  BOOLEAN                      Updated;

  if (Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

//  LocateHiiProtocols ();
   Status = pBS->LocateProtocol(&gEfiHiiDatabaseProtocolGuid, NULL, &HiiDatabase);
   //HiiDatabase = gIfrLibHiiDatabase;

  //
  // Get the orginal package list
  //
  BufferSize = 0;
  HiiPackageList   = NULL;
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, Handle, &BufferSize, HiiPackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HiiPackageList = Malloc (BufferSize);
    ASSERT (HiiPackageList != NULL);

    Status = HiiDatabase->ExportPackageLists (HiiDatabase, Handle, &BufferSize, HiiPackageList);
    if (EFI_ERROR (Status)) {
      pBS->FreePool (HiiPackageList);
      return Status;
    }
  }

  //
  // Calculate and allocate space for retrieval of IFR data
  //
  BufferSize += Data->Offset;
  UpdateBuffer = Malloc (BufferSize);
  if (UpdateBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  UpdateBufferPos = (UINT8 *) UpdateBuffer;

  //
  // copy the package list header
  //
  pBS->CopyMem (UpdateBufferPos, HiiPackageList, sizeof (EFI_HII_PACKAGE_LIST_HEADER));
  UpdateBufferPos += sizeof (EFI_HII_PACKAGE_LIST_HEADER);

  Updated = FALSE;
  for (Index = 0; ; Index++) {
    Status = GetPackageDataFromPackageList (HiiPackageList, Index, &PackageLength, &Package);
    if (Status == EFI_SUCCESS) {
      pBS->CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));
      if ((PackageHeader.Type == EFI_HII_PACKAGE_FORMS) && !Updated) {
        Status = UpdateFormPackageData (FormSetGuid, FormId, Package, PackageLength, Label, Insert, Data, (UINT8 **)&TempBuffer, &TempBufferSize);
        if (!EFI_ERROR(Status)) {
          if (FormSetGuid == NULL) {
            Updated = TRUE;
          }
          pBS->CopyMem (UpdateBufferPos, TempBuffer, TempBufferSize);
          UpdateBufferPos += TempBufferSize;
          pBS->FreePool (TempBuffer);
          continue;
        }
      }

      pBS->CopyMem (UpdateBufferPos, Package, PackageLength);
      UpdateBufferPos += PackageLength;
    } else if (Status == EFI_NOT_FOUND) {
      break;
    } else {
      pBS->FreePool (HiiPackageList);
      return Status;
    }
  }

  //
  // Update package list length
  //
  BufferSize = UpdateBufferPos - (UINT8 *) UpdateBuffer;
  pBS->CopyMem (&UpdateBuffer->PackageLength, &BufferSize, sizeof (UINT32));

  pBS->FreePool (HiiPackageList);

  Status = HiiDatabase->UpdatePackageList (HiiDatabase, Handle, UpdateBuffer);

  pBS->FreePool(UpdateBuffer);

  return Status;
}
#endif	//#if EFI_SPECIFICATION_VERSION>0x20000

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	CreateHiiOneOfItem
//
// Description:	Create a Hii OneOf Item.
//
// Input:
//	UINT8 **IfrBufferPtr, 
//	UINT16 VarStoreId, 
//	UINT16 VarOffset, 
//	UINT16 CallBackKey, 
//	STRING_REF StrTokenId, 
//	STRING_REF StrTokenHelpId, 
//	UINT8 Flags
//
// Output:
//	EFI_STATUS
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateHiiOneOfItem(
	UINT8 **IfrBufferPtr, 
	UINT16 VarStoreId, 
	UINT16 VarOffset, 
	UINT16 CallBackKey, 
	STRING_REF StrTokenId, 
	STRING_REF StrTokenHelpId, 
	UINT8 Flags)
{
	EFI_IFR_ONE_OF IfrOneOf;

	IfrOneOf.Header.OpCode = EFI_IFR_ONE_OF_OP;
	IfrOneOf.Header.Length = sizeof(EFI_IFR_ONE_OF);
	IfrOneOf.Header.Scope = TRUE;
	IfrOneOf.Question.Header.Prompt = StrTokenId;
	IfrOneOf.Question.Header.Help = StrTokenHelpId;
	IfrOneOf.Question.QuestionId = CallBackKey;	//CallBack Key.
	IfrOneOf.Question.VarStoreId = VarStoreId;	//Variable ID
	IfrOneOf.Question.VarStoreInfo.VarOffset = VarOffset;
	IfrOneOf.Question.Flags = 04; 			//EFI_IFR_FLAG_INTERACTIVE
	IfrOneOf.Flags = Flags;
	IfrOneOf.data.u64.MinValue = 0;
	IfrOneOf.data.u64.MaxValue = 0;
	IfrOneOf.data.u64.Step = 0;

	pBS->CopyMem( *IfrBufferPtr, &IfrOneOf, sizeof(EFI_IFR_ONE_OF) );
	*IfrBufferPtr += sizeof(EFI_IFR_ONE_OF);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	CreateHiiOneOfOptionItem
//
// Description:	Create a Hii OneOf Option Item.
//
// Input:
//  	UINT8 **IfrBufferPtr
//	UINT8 Type 
//	UINT16 Index
//	STRING_REF StrTokenId
//
// Output:
//	EFI_STATUS
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateHiiOneOfOptionItem(UINT8 **IfrBufferPtr, UINT8 Type, UINT16 Index, STRING_REF StrTokenId)
{
	EFI_IFR_ONE_OF_OPTION IfrOneOfOptiopn;

	IfrOneOfOptiopn.Header.OpCode = EFI_IFR_ONE_OF_OPTION_OP;
	IfrOneOfOptiopn.Header.Length = sizeof(EFI_IFR_ONE_OF_OPTION);
	IfrOneOfOptiopn.Header.Scope = FALSE;
	IfrOneOfOptiopn.Option = StrTokenId;
	IfrOneOfOptiopn.Flags = 0;
  	IfrOneOfOptiopn.Type = Type;

	switch( Type )
	{
		case 0:
  			IfrOneOfOptiopn.Value.u8 = (UINT8)Index;	
			break;
		case 1:
  			IfrOneOfOptiopn.Value.u16 = Index;	
			break;

	}

	pBS->CopyMem ( *IfrBufferPtr, &IfrOneOfOptiopn, sizeof(EFI_IFR_ONE_OF_OPTION) );
	*IfrBufferPtr += sizeof(EFI_IFR_ONE_OF_OPTION);
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	CreateHiiRefItem
//
// Description:	Create a Hii Ref Item.
//
// Input:
//  	UINT8 **IfrBufferPtr
//	UINT16 FromID
//	STRING_REF StrTokenId1
//	STRING_REF StrTokenId2
//
// Output:
//	EFI_STATUS
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateHiiRefItem(UINT8 **IfrBufferPtr, UINT16 FromID, STRING_REF StrTokenId1, STRING_REF StrTokenId2)
{
	EFI_IFR_REF IfrRef;

	IfrRef.Header.OpCode = EFI_IFR_REF_OP;
	IfrRef.Header.Length = sizeof(EFI_IFR_REF);
	IfrRef.Header.Scope = FALSE;
	IfrRef.Question.Header.Prompt = StrTokenId1;
	IfrRef.Question.Header.Help = StrTokenId2;
	IfrRef.Question.QuestionId = 0x10;		//CallBack Key.
	IfrRef.Question.VarStoreId = 0;		//Setup Variable ID
	IfrRef.Question.VarStoreInfo.VarOffset = 0xFFFF;
	IfrRef.Question.Flags = 00; 			//INTERACTIVE;
	IfrRef.FormId =	FromID;
			
	pBS->CopyMem ( *IfrBufferPtr, &IfrRef, sizeof(EFI_IFR_REF) );
	*IfrBufferPtr += sizeof(EFI_IFR_REF);
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	CreateHiiIfrEndItem
//
// Description:	Create a Ifr End Item.
//
// Input:
//  	UINT8 **IfrBufferPtr
// Output:
//	EFI_STATUS
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateHiiIfrEndItem(UINT8 **IfrBufferPtr)
{
	EFI_IFR_END IfrEnd;

	IfrEnd.Header.OpCode = EFI_IFR_END_OP;
	IfrEnd.Header.Length = sizeof(EFI_IFR_END);
	IfrEnd.Header.Scope = FALSE;

	pBS->CopyMem( *IfrBufferPtr, &IfrEnd, sizeof(EFI_IFR_END) );
	*IfrBufferPtr += sizeof(EFI_IFR_END);
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	CreateHiiSubtitleItem
//
// Description:	Create a Subtitle Item.
//
// Input:
//  	UINT8 **IfrBufferPtr
//	STRING_REF StrTokenId
//	STRING_REF HelpStrTokenId
//	UINT8 Flags
//
// Output:
//	EFI_STATUS
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateHiiSubtitleItem(UINT8 **IfrBufferPtr, STRING_REF StrTokenId, STRING_REF HelpStrTokenId, UINT8 Flags )
{
	EFI_IFR_SUBTITLE IfrSubtitle;
	IfrSubtitle.Header.OpCode = EFI_IFR_SUBTITLE_OP;
	IfrSubtitle.Header.Length = sizeof(EFI_IFR_SUBTITLE);
	IfrSubtitle.Header.Scope = TRUE;
	IfrSubtitle.Statement.Prompt = StrTokenId;
	IfrSubtitle.Statement.Help = HelpStrTokenId;
	IfrSubtitle.Flags = Flags;

	pBS->CopyMem( *IfrBufferPtr, &IfrSubtitle, sizeof(EFI_IFR_SUBTITLE) );
	*IfrBufferPtr += sizeof(EFI_IFR_SUBTITLE);
	return EFI_SUCCESS;
};

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	CreateHiiIfrFormItem
//
// Description:	Create a Form Item.
//
// Input:
//  	UINT8 **IfrBufferPtr
//	UINT16 FromID
//	STRING_REF StrTokenId
//
// Output:
//	EFI_STATUS
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateHiiIfrFormItem(UINT8 **IfrBufferPtr, UINT16 FromID, STRING_REF StrTokenId)
{
	EFI_IFR_FORM IfrForm;

	IfrForm.Header.OpCode = EFI_IFR_FORM_OP;
	IfrForm.Header.Length = sizeof(EFI_IFR_FORM);
	IfrForm.Header.Scope = TRUE;
	IfrForm.FormId = FromID;
	IfrForm.FormTitle = StrTokenId;

	pBS->CopyMem( *IfrBufferPtr, &IfrForm, sizeof(EFI_IFR_FORM) );
	*IfrBufferPtr += sizeof(EFI_IFR_FORM);
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	InitHiiData
//
// Description:	Use HiiDatabaseProtocol get hii data package to buffer
//
// Input:
//  	EFI_HANDLE HiiHandle
//
// Output:
//	NONE
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
void InitHiiData(EFI_HANDLE HiiHandle)
{

EFI_GUID gEfiHiiDatabaseProtocolGuid = EFI_HII_DATABASE_PROTOCOL_GUID;
EFI_HII_DATABASE_PROTOCOL *gHiiDatabase;
EFI_STATUS status;


	status = pBS->LocateProtocol(&gEfiHiiDatabaseProtocolGuid, NULL, &gHiiDatabase);
        if(EFI_ERROR(status)) return;
	hiiFormPackage = Malloc(FormPackageLength);
	status = gHiiDatabase->ExportPackageLists(gHiiDatabase, HiiHandle, &FormPackageLength, hiiFormPackage);
	TRACE((-1,"[FBOHii.c] ExportPackageLists...%r\n",  status));	

	if( status == EFI_BUFFER_TOO_SMALL)
	{

		pBS->FreePool(hiiFormPackage);
		//
		// Allocate space for retrieval of IFR data
		//
		hiiFormPackage = Malloc(FormPackageLength);

		if (hiiFormPackage == NULL)
		{
			hiiFormPackage = NULL;
			return;
		}
		//
		// Get all the packages associated with this HiiHandle
		//
		status = gHiiDatabase->ExportPackageLists (gHiiDatabase, HiiHandle, &FormPackageLength, hiiFormPackage);
		TRACE((-1,"[FBOHii.c] Again ExportPackageLists...%r size=%x\n",  status, FormPackageLength));	
//		TraceOutData( (UINT8 *)hiiFormPackage, FormPackageLength);
	}

}

void FreeHiiData()
{
	if(hiiFormPackage)
		pBS->FreePool(hiiFormPackage);
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	FIndVariableID
//
// Description:	Search Hii Form Package Data of variable ID.
//
// Input: 
//	CHAR8 *VarName
// Output: 
//	UINT16 VarID
// Modified:
//      NONE
// Referrals: 
//	NONE
// Notes:	
//	NONE
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 FIndVariableID(CHAR8 *VarName )
{
	EFI_HII_PACKAGE_HEADER *pkgHdr;
	

	UINT32 offset = 0;

	EFI_IFR_OP_HEADER *IfrData;
	UINT32 SeekSize=0;
	UINT32 FormSize=0;

	if( hiiFormPackage == NULL ) return 0;

  	offset = sizeof(EFI_HII_PACKAGE_LIST_HEADER);

  	while( offset < ((EFI_HII_PACKAGE_LIST_HEADER*)hiiFormPackage)->PackageLength )
  	{
                UINT8 *pkgOffset;
    		pkgOffset = ((UINT8*)hiiFormPackage) + offset;
    		pkgHdr = (EFI_HII_PACKAGE_HEADER*)pkgOffset;

		TRACE((-1,"[FixedBootOrderHII.c] pkgHdr=%x Type=%x Length=%x\n", (UINT8*)pkgHdr-(UINT8*)hiiFormPackage,
									   pkgHdr->Type,
									   pkgHdr->Length ));

    		switch(pkgHdr->Type)
    		{
    			case EFI_HII_PACKAGE_FORMS:
				IfrData = (EFI_IFR_OP_HEADER*) ((UINT8 *)pkgHdr+sizeof(EFI_HII_PACKAGE_HEADER));
				FormSize=pkgHdr->Length;
				while(1)
				{
					if(IfrData->OpCode == EFI_IFR_VARSTORE_OP)
					{

//						TRACE((-1,"[FixedBootOrderHii.c] VarID=%x Name=%s\n", ((EFI_IFR_VARSTORE*)IfrData)->VarStoreId,
//												      ((EFI_IFR_VARSTORE*)IfrData)->Name ));


						if( !Strcmp(VarName, ((EFI_IFR_VARSTORE*)IfrData)->Name) )
							return ((EFI_IFR_VARSTORE*)IfrData)->VarStoreId;
					}
					SeekSize+=IfrData->Length;		
					(UINT8 *)IfrData+=IfrData->Length;

					if(!IfrData->Length) break;
					if(SeekSize >= FormSize) break;
				}
      				return 0;
      				break;

			case EFI_HII_PACKAGE_END:
				return 0;
				break;

    			default:
     	 			break;
    		}
    		offset += pkgHdr->Length;
  	}
	return 0;
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
