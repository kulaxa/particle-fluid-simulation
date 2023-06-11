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

# Filter FPS values in the range 200 to 0
filtered_particles = []
filtered_fps = []
uppter_bound = 300
for p, f in zip(particles, fps):
    if 0 <= f <= uppter_bound:
        filtered_particles.append(p)
        filtered_fps.append(f)

# Plot the graph
plt.scatter(filtered_particles, filtered_fps)
plt.xlabel("Particle Count")
plt.ylabel("FPS")
plt.title(f"Particle Count vs. FPS (Grid Size: {grid_size}) (Range: 0-{uppter_bound})")

# title = "Particle Count vs. FPS (Range: {}-0)".format(uppter_bound)
# plt.title(title)
plt.grid(True)
file_name = 'figure' + str(uppter_bound) + '_' + str(grid_size) + '.png'
plt.savefig(file_name)