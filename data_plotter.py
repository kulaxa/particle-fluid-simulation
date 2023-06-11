import matplotlib.pyplot as plt

# Read data from file
data = []
with open("results.txt", "r") as file:
    grid_size = int(file.readline().split(":")[1].strip())  # Read and extract the grid size
    for line in file:
        if ":" in line:
            particle, fps = line.split(":")
            data.append((int(particle.strip()), float(fps.strip())))

# Separate particles and FPS values
particles = [d[0] for d in data]
fps = [d[1] for d in data]

# Plot the graph
plt.scatter(particles, fps)
plt.xlabel("Particle Count")
plt.ylabel("FPS")
plt.title(f"Particle Count vs. FPS (Grid Size: {grid_size})")
plt.grid(True)
file_name = 'figure-unlimited_' + str(grid_size) + '.png'

plt.savefig(file_name)