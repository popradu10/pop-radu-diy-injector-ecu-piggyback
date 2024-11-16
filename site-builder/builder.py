# Install the markdown library if you haven't already:
# pip install markdown

import markdown

# Read the Markdown content from a file
input_file_path = "README.md"  # Path to your Markdown file
output_file_path = "index.html"  # Path for the output HTML file

# Read Markdown content from the file
with open(input_file_path, "r", encoding="utf-8") as file:
    markdown_content = file.read()

# Convert Markdown to HTML
html_content = markdown.markdown(markdown_content)

# Save the HTML content to a new file
with open(output_file_path, "w", encoding="utf-8") as file:
    file.write(html_content)