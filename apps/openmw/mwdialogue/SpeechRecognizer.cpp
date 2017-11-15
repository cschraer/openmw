/////////////////////////////////////////////////////////////////////////////
// Speech recognizer
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

#include "FrotzApp.h"
#include "FrotzWnd.h"

#include "SpeechRecognizer.h"

#include <string>
#include <Windows.Foundation.h>
#include <Windows.Media.SpeechRecognition.h>
#include <Windows.System.Threading.h>

#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <wrl\event.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::System::Threading;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

bool SpeechRecognizer::Initialize()
{
	// Get the activation factory for the ISpeechRecognizerFactory interface.
	ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizerFactory> speechRecognizerFactory;
	HRESULT hr = GetActivationFactory(
		HStringReference(RuntimeClass_Windows_Media_SpeechRecognition_SpeechRecognizer).Get(),
		&speechRecognizerFactory);

	if (FAILED(hr))
	{
		// TODO: Log failure.
		return false;
	}

	// Create a speech recognizer.
	hr = speechRecognizerFactory->Create(nullptr, &mSpeechRecognizer);

	if (FAILED(hr))
	{
		// TODO: Log failure.
		return false;
	}

	// Configure the speech UI.
	ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizerUIOptions> uiOptions;
	hr = mSpeechRecognizer->get_UIOptions(&uiOptions);

	if (FAILED(hr))
	{
		// TODO: Log failure.
		return false;
	}

	uiOptions->put_IsReadBackEnabled(false);		// TODO: Do I need to check HRESULT?
	uiOptions->put_ShowConfirmation(false);			// TODO: Do I need to check HRESULT?

	// Compile dictation grammar once at start up.
	ComPtr<IAsyncOperation<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*>> compileOp;
	hr = mSpeechRecognizer->CompileConstraintsAsync(&compileOp);

	if (FAILED(hr))
	{
		// TODO: Log failure.
		return false;
	}

	hr = compileOp->put_Completed(
		Callback<IAsyncOperationCompletedHandler<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*>>(
			[this]
			(
				IAsyncOperation<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*> *op,
				AsyncStatus status
			) -> HRESULT
	{
		if (status == AsyncStatus::Completed)
		{
			// TODO: Check that compilation result was OK.
			this->mInitialized = true;
		}

		return (status != AsyncStatus::Error ? S_OK : E_FAIL);
	}).Get());

	if (FAILED(hr))
	{
		// TODO: Log failure.
		return false;
	}

	return true;
}

bool SpeechRecognizer::StartListening(FrotzWnd * pWindow)
{
	if (!mInitialized)
	{
		return false;
	}

	ComPtr<IAsyncOperation<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionResult*>> recoOp;
	HRESULT hr = mSpeechRecognizer->RecognizeWithUIAsync(&recoOp);

	if (FAILED(hr))
	{
		// TODO: Log failure.
		return false;
	}

	hr = recoOp->put_Completed(
		Callback<IAsyncOperationCompletedHandler<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionResult*>>(
			[this, pWindow]
			(
				IAsyncOperation<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionResult*> *op,
				AsyncStatus status
			) -> HRESULT
	{
		// Get the speech reco text if the async listen operation completed succesfully.
		if (status == AsyncStatus::Completed)
		{
			// Get text.
			ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognitionResult> recoResult;
			HRESULT hr2 = op->GetResults(&recoResult);

			if (FAILED(hr2))
			{
				// TODO: Handle errors.
				return hr2;
			}

			auto resultText = GetRecoResultText(recoResult);

			if (resultText.size() > 0)
			{
				pWindow->InputString(resultText);
				pWindow->InputZcodeKey(ZC_RETURN);
			}
		}

		return (status != AsyncStatus::Error ? S_OK : E_FAIL);
	}).Get());

	if (FAILED(hr))
	{
		// TODO: Log failure.
		return false;
	}

	return true;
}

std::wstring SpeechRecognizer::GetRecoResultText(
	const ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognitionResult>& recoResult) const
{
	HSTRING text;
	auto hr = recoResult->get_Text(&text);

	if (FAILED(hr))
	{
		return L"SPEECH RECOGNITION FAILED!";
	}

	return std::wstring(WindowsGetStringRawBuffer(text, nullptr));
}