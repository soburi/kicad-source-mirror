///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug  2 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_rescue_each_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_RESCUE_EACH_BASE::DIALOG_RESCUE_EACH_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 400,-1 ), wxDefaultSize );
	
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );
	
	m_htmlPrompt = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	bSizerMain->Add( m_htmlPrompt, 1, wxALL|wxEXPAND, 5 );
	
	m_titleSymbols = new wxStaticText( this, wxID_ANY, _("Symbols to update:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_titleSymbols->Wrap( -1 );
	m_titleSymbols->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizerMain->Add( m_titleSymbols, 0, wxLEFT|wxRIGHT, 5 );
	
	m_ListOfConflicts = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerMain->Add( m_ListOfConflicts, 2, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_titleInstances = new wxStaticText( this, wxID_ANY, _("Instances of this symbol:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_titleInstances->Wrap( -1 );
	m_titleInstances->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizerMain->Add( m_titleInstances, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_ListOfInstances = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerMain->Add( m_ListOfInstances, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizerView;
	bSizerView = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, _("Cached Symbol:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	m_staticText2->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer6->Add( m_staticText2, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_componentViewOld = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE|wxFULL_REPAINT_ON_RESIZE );
	m_componentViewOld->SetMinSize( wxSize( 150,150 ) );
	
	bSizer6->Add( m_componentViewOld, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	bSizerView->Add( bSizer6, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, _("Library Symbol:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	m_staticText3->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer7->Add( m_staticText3, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_componentViewNew = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE|wxFULL_REPAINT_ON_RESIZE );
	m_componentViewNew->SetMinSize( wxSize( 150,150 ) );
	
	bSizer7->Add( m_componentViewNew, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	bSizerView->Add( bSizer7, 1, wxEXPAND, 5 );
	
	
	bSizerMain->Add( bSizerView, 2, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	m_btnNeverShowAgain = new wxButton( this, wxID_ANY, _("Never Show Again"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_btnNeverShowAgain, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer5->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_stdButtons = new wxStdDialogButtonSizer();
	m_stdButtonsOK = new wxButton( this, wxID_OK );
	m_stdButtons->AddButton( m_stdButtonsOK );
	m_stdButtonsCancel = new wxButton( this, wxID_CANCEL );
	m_stdButtons->AddButton( m_stdButtonsCancel );
	m_stdButtons->Realize();
	
	bSizer5->Add( m_stdButtons, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizerMain->Add( bSizer5, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizerMain );
	this->Layout();
	bSizerMain->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( DIALOG_RESCUE_EACH_BASE::OnDialogResize ) );
	m_ListOfConflicts->Connect( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( DIALOG_RESCUE_EACH_BASE::OnConflictSelect ), NULL, this );
	m_componentViewOld->Connect( wxEVT_PAINT, wxPaintEventHandler( DIALOG_RESCUE_EACH_BASE::OnHandleCachePreviewRepaint ), NULL, this );
	m_componentViewNew->Connect( wxEVT_PAINT, wxPaintEventHandler( DIALOG_RESCUE_EACH_BASE::OnHandleLibraryPreviewRepaint ), NULL, this );
	m_btnNeverShowAgain->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_RESCUE_EACH_BASE::OnNeverShowClick ), NULL, this );
	m_stdButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_RESCUE_EACH_BASE::OnCancelClick ), NULL, this );
}

DIALOG_RESCUE_EACH_BASE::~DIALOG_RESCUE_EACH_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( DIALOG_RESCUE_EACH_BASE::OnDialogResize ) );
	m_ListOfConflicts->Disconnect( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( DIALOG_RESCUE_EACH_BASE::OnConflictSelect ), NULL, this );
	m_componentViewOld->Disconnect( wxEVT_PAINT, wxPaintEventHandler( DIALOG_RESCUE_EACH_BASE::OnHandleCachePreviewRepaint ), NULL, this );
	m_componentViewNew->Disconnect( wxEVT_PAINT, wxPaintEventHandler( DIALOG_RESCUE_EACH_BASE::OnHandleLibraryPreviewRepaint ), NULL, this );
	m_btnNeverShowAgain->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_RESCUE_EACH_BASE::OnNeverShowClick ), NULL, this );
	m_stdButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_RESCUE_EACH_BASE::OnCancelClick ), NULL, this );
	
}
