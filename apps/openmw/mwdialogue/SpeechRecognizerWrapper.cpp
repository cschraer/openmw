/////////////////////////////////////////////////////////////////////////////
// Speech recognizer
/////////////////////////////////////////////////////////////////////////////
//#include "StdAfx.h"

//#include "FrotzApp.h"
//#include "FrotzWnd.h"


#include <ppltasks.h>
#include "SpeechRecognizerWrapper.hpp"

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
using namespace ABI::Windows::Media::SpeechRecognition;

bool SpeechRecognizerWrapper::Initialize(bool continuous)
{
    // Get the activation factory for the ISpeechRecognizerFactory interface.
    ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizerFactory> speechRecognizerFactory;
    HRESULT hr = GetActivationFactory(
        HStringReference(RuntimeClass_Windows_Media_SpeechRecognition_SpeechRecognizer).Get(),
        &speechRecognizerFactory);

    ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizer> sr;

    if (FAILED(hr))
    {
        // TODO: Log failure.
        return false;
    }

    // Create a speech recognizer.
    hr = speechRecognizerFactory->Create(nullptr, &sr);

    if (FAILED(hr))
    {
        // TODO: Log failure.
        return false;
    }

    // Compile dictation grammar once at start up.
    ComPtr<IAsyncOperation<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*>> compileOp;
    hr = sr->CompileConstraintsAsync(&compileOp);

    if (FAILED(hr))
    {
        // TODO: Log failure.
        return false;
    }

    hr = compileOp->put_Completed(
        MakeAgileCallback<IAsyncOperationCompletedHandler<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*>>(
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
            }
        ).Get());

    if (continuous)
    {
        hr = sr.As<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizer2>(&mSpeechRecognizer);

        if (FAILED(hr))
        {
            // TODO: Log failure.
            return false;
        }

        hr = mSpeechRecognizer->get_ContinuousRecognitionSession(&mSpeechRecognitionSession);
    }


    if (FAILED(hr))
    {
        // TODO: Log failure.
        return false;
    }

    return true;
}

bool SpeechRecognizerWrapper::RegisterForContinousResults(const std::function<void(std::wstring)>& callback)
{
    auto resultGeneratedHandler =
        MakeAgileCallback<ITypedEventHandler<SpeechContinuousRecognitionSession*, SpeechContinuousRecognitionResultGeneratedEventArgs*>>(
            [this, callback](ISpeechContinuousRecognitionSession* session, ISpeechContinuousRecognitionResultGeneratedEventArgs* args) mutable -> HRESULT
    {
        ComPtr<ISpeechRecognitionResult> result;
        HRESULT hr = args->get_Result(&result);

        if (FAILED(hr))
        {
            //TODO: log failure
            return hr;
        }

        callback(GetRecoResultText(result));

        return S_OK;
    });

    HRESULT hr = mSpeechRecognitionSession->add_ResultGenerated(resultGeneratedHandler.Get(),  &mResultToken);

    if (FAILED(hr))
    {
        // TODO: Log failure.
        return false;
    }
    return true;
}

bool SpeechRecognizerWrapper::UnRegisterForContinousResults()
{
    HRESULT hr = mSpeechRecognitionSession->remove_ResultGenerated(mResultToken);

    if (FAILED(hr))
    {
        // TODO: Log failure.
        return false;
    }
    return true;
}

bool SpeechRecognizerWrapper::StartContinuousListening()
{
    ComPtr<IAsyncAction> startAction;
    HRESULT hr = mSpeechRecognitionSession->StartAsync(&startAction);

    if (FAILED(hr))
    {
        //TODO: log failure
        return false;
    }

    hr = startAction->put_Completed(
        MakeAgileCallback<IAsyncActionCompletedHandler>(
            [this]
    (
        IAsyncAction *op,
        AsyncStatus status
        ) -> HRESULT
    {
        if (status == AsyncStatus::Completed)
        {
            // TODO: Check that compilation result was OK.
            this->mContinuouslyListening = true;
        }

        return (status != AsyncStatus::Error ? S_OK : E_FAIL);
    }
    ).Get());

    int count = 0;
    while (!mContinuouslyListening && count < 10)
    {
        Sleep(1000);
        count++;
    }

    if (!mContinuouslyListening)
    {
        return false;
    }

    return true;
}

bool SpeechRecognizerWrapper::StartListening(const std::function<void(std::wstring)>& callback)
{
    int count = 0;
    while (!mInitialized && count < 10)
    {
        Sleep(1000);
        count++;
    }

    if (!mInitialized) 
    {
        return false;
    }

    ComPtr<IAsyncOperation<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionResult*>> recoOp;
    Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizer> sr;

    HRESULT hr = mSpeechRecognizer.As<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizer>(&sr);
    hr = sr->RecognizeAsync(&recoOp);

    if (FAILED(hr))
    {
        // TODO: Log failure.
        return false;
    }

    hr = recoOp->put_Completed(
        Callback<IAsyncOperationCompletedHandler<ABI::Windows::Media::SpeechRecognition::SpeechRecognitionResult*>>(
            [this, callback]
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
                callback(resultText);
                //pWindow->InputZcodeKey(ZC_RETURN);
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

std::wstring SpeechRecognizerWrapper::GetRecoResultText(
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