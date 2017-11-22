/////////////////////////////////////////////////////////////////////////////
// Speech recognizer
/////////////////////////////////////////////////////////////////////////////
#ifndef SpeechRecognizerWrapper_H
#define SpeechRecognizerWrapper_H

#pragma once
#include <Windows.Foundation.h>
#include <Windows.Media.SpeechRecognition.h>
#include <string>
#include <functional>

#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>

class FrotzWnd;


//
// Creates an agile callback based on the ::Microsoft::WRL::Callback helper class
//
template<typename TDelegateInterface, typename TCallback>
::Microsoft::WRL::ComPtr<TDelegateInterface> MakeAgileCallback(TCallback callback) throw()
{
    return ::Microsoft::WRL::Callback<
        ::Microsoft::WRL::Implements<
        ::Microsoft::WRL::RuntimeClassFlags<::Microsoft::WRL::ClassicCom>,
        TDelegateInterface,
        ::Microsoft::WRL::FtmBase>>(callback);
}

class SpeechRecognizerWrapper
{
public:
    bool Initialize(bool continuous);
    bool IsInitialized() const { return mInitialized; }

    bool RegisterForContinousResults(const std::function<void(std::wstring)>& callback);
    bool UnRegisterForContinousResults();

    bool StartContinuousListening();
    bool StartListening(const std::function<void(std::wstring)>& callback);

    std::wstring GetRecoResultText(
        const Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognitionResult>& recoResult) const;

private:
    bool mInitialized = false;
    bool mContinuouslyListening = false;
    EventRegistrationToken mResultToken;
    Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizer2> mSpeechRecognizer;
    Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechContinuousRecognitionSession> mSpeechRecognitionSession;
};

#endif