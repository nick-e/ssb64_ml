import sys
import model

def main():
	if len(sys.argv) >= 7:
		model.save_model(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]))

if __name__ == "__main__":
	main()
