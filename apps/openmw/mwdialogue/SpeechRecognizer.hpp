/////////////////////////////////////////////////////////////////////////////
// Speech recognizer
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <Windows.Foundation.h>
#include <Windows.Media.SpeechRecognition.h>
#include <string>

#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>

class FrotzWnd;

class SpeechRecognizer
{
public:
	bool Initialize();
	bool IsInitialized() const { return mInitialized; }

	// TODO: Don't take FrotzApp as a callback instead make the text result a promise or something
	// that can be used by the callsite.
	bool StartListening(FrotzWnd * pWindow);

private:
	std::wstring GetRecoResultText(
		const Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognitionResult>& recoResult) const;

private:
	bool mInitialized = false;
	Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechRecognition::ISpeechRecognizer> mSpeechRecognizer;
};