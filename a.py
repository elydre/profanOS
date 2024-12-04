import subprocess

def run_and_test_aout(input_number, expected_output):
	"""
	Exécute `a.out` avec un nombre donné et vérifie si la sortie correspond à celle attendue.

	:param input_number: Nombre entier à factoriser.
	:param expected_output: Sortie attendue sous forme de chaîne (ex: "2*2*3*7").
	:return: Booléen indiquant si le test a réussi ou non.
	"""
	try:
		# Exécution du fichier a.out avec l'entrée spécifiée
		process = subprocess.run(
			['./a.out', str(input_number)],
			text=True,  # Permet de manipuler les chaînes de caractères
			capture_output=True,  # Capture la sortie standard et d'erreur
			check=True  # Lève une exception si le code de retour n'est pas 0
		)

		# Récupération de la sortie standard
		actual_output = process.stdout.strip()

		# Comparaison de la sortie avec celle attendue
		if actual_output == expected_output:
			print(f"Test réussi pour {input_number}: {actual_output}")
			return True
		else:
			print(f"Test échoué pour {input_number}: attendu '{expected_output}', obtenu '{actual_output}'")
			return False

	except subprocess.CalledProcessError as e:
		print(f"Erreur lors de l'exécution de a.out: {e}")
		return False
	except FileNotFoundError:
		print("Erreur : Le fichier `a.out` est introuvable.")
		return False

def is_prime(n):
	if n <= 1:
		return False
	if n <= 3:
		return True
	if n % 2 == 0 or n % 3 == 0:
		return False
	i = 3
	while i * i <= n:
		if n % i == 0:
			return False
		i += 2
	return True

def prime_factors(n):
	if (is_prime(n)):
		return [n]
	i = 2
	factors = []
	while i * i <= n:
		if n % i:
			i += 1
		else:
			n //= i
			factors.append(i)
	if n > 1:
		factors.append(n)
	return factors

## Exemple d'utilisation
#tests = [
#	(i, '*'.join(map(str, prime_factors(i)))) for i in range(200000, 400000)
#]
#
## Exécute tous les tests
#for number, expected in tests:
#	if (not run_and_test_aout(number, expected)):
#		exit(1)

# find who has the most prime factors
who = 1
factors = [1]
for i in range(0, 100000):
	new_factors = prime_factors(i)
	if len('*'.join(map(str,new_factors))) > len('*'.join(map(str,factors))):
		factors = new_factors
		who = i

print(who)
print('*'.join(map(str, factors)))
