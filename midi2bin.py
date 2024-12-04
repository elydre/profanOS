import mido
import struct
import argparse

# Convertit une note MIDI en fréquence (Hz)
def midi_note_to_frequency(note):
    return 440.0 * (2 ** ((note - 69) / 12.0))

# Lit un fichier MIDI monophonique et extrait les notes et durées
def parse_midi_file(input_midi_path):
    midi_data = mido.MidiFile(input_midi_path)
    events = []
    current_time = 0
    last_note = None

    for msg in midi_data:
        current_time += msg.time
        if msg.type == 'note_on' and msg.velocity > 0:
            if last_note is not None:  # Ajoute la note précédente
                duration_ms = int(current_time * 1000)
                frequency = midi_note_to_frequency(last_note)
                events.append((frequency, duration_ms))
            last_note = msg.note
            current_time = 0
        elif msg.type in ('note_off', 'note_on') and msg.velocity == 0:
            if last_note is not None:
                duration_ms = int(current_time * 1000)
                frequency = midi_note_to_frequency(last_note)
                events.append((frequency, duration_ms))
                last_note = None
                current_time = 0
        elif msg.type == 'note_on' and msg.velocity == 0:  # Pause
            if current_time > 0 and last_note is None:
                duration_ms = int(current_time * 1000)
                events.append((0, duration_ms))  # Pause avec fréquence 0
                current_time = 0

    return events

# Convertit les événements MIDI en fichier binaire
def write_binary_output(events, output_binary_path):
    with open(output_binary_path, 'wb') as f:
        for frequency, duration in events:
            f.write(struct.pack('fI', frequency, duration))

def main():
    # Analyse des arguments de ligne de commande
    parser = argparse.ArgumentParser(description="Convertir un fichier MIDI monophonique en fichier binaire.")
    parser.add_argument('input', type=str, help="Chemin du fichier MIDI d'entrée.")
    parser.add_argument('output', type=str, help="Chemin du fichier binaire de sortie.")
    args = parser.parse_args()

    # Conversion
    print(f"Traitement du fichier MIDI : {args.input}")
    events = parse_midi_file(args.input)
    write_binary_output(events, args.output)
    print(f"Fichier binaire généré : {args.output}")

if __name__ == '__main__':
    main()
