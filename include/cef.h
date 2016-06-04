#ifndef _CEF_TERA_H_
#define _CEF_TERA_H_

#include <include\cef_app.h>
#include <include\cef_browser.h>
#include <include\cef_client.h>

#include "Handler.h"

class CCefV8Handler : public CefV8Handler
{
private:
	IMPLEMENT_REFCOUNTING(CCefV8Handler);

public:
	virtual bool Execute(const CefString& name,
		CefRefPtr<CefV8Value> object,
		const CefV8ValueList& arguments,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) OVERRIDE;
};

class CCefApp : 
	public CefApp,		
	public CefBrowserProcessHandler,
	public CefRenderProcessHandler
{
private:
	IMPLEMENT_REFCOUNTING(CCefApp);

protected:
	CefRefPtr<CefBrowser> m_MainBrowser = NULL;

public:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE 
	{
		return this;
	}

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE 
	{
		return this;
	}

	virtual bool OnProcessMessageReceived(
		CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message) OVERRIDE;

	virtual void OnWebKitInitialized() OVERRIDE;
	virtual void OnContextInitialized() OVERRIDE;
	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;

	CefRefPtr<CefBrowser> GetBrowser()
	{
		return m_MainBrowser;
	}
};

static void DoCefShutdown()
{
	if (!CCefHandler::GetInstance())
	{
		return;
	}

	CefQuitMessageLoop();

	CCefHandler::GetInstance()->Close();
	CCefHandler::GetInstance()->GetRequestHandler()->Release();

	CefShutdown();
}

static int CefExecute(HINSTANCE hInstance, CefRefPtr<CCefApp> pHandler)
{
	CefMainArgs main_args(hInstance);

	void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
#endif

	return CefExecuteProcess(main_args, pHandler.get(), sandbox_info);
}

static int CefInit(HINSTANCE hInstance, CefRefPtr<CCefApp> pHandler)
{
	CefMainArgs main_args(hInstance);

	void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
#endif

	CefSettings settings;
#ifdef CEF_SINGLE_PROCESS
	settings.single_process = true;
#endif
	settings.multi_threaded_message_loop = true;
#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif

	if (!CefInitialize(main_args, settings, pHandler.get(), sandbox_info))
	{
		return -1;
	}

	return 1;
}

extern void CefRun(HWND hWnd);

#endif