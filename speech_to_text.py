# speech_to_text.py
import speech_recognition as sr

r = sr.Recognizer()

with sr.Microphone() as source:
    print(" Listening...")
    try:
        audio = r.listen(source, timeout=5)
        text = r.recognize_google(audio)
        print(" You said:", text)
        with open("speech_input.txt", "w") as f:
            f.write(text)
    except:
        with open("speech_input.txt", "w") as f:
            f.write("")
