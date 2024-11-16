# Install the markdown library if you haven't already:
# pip install markdown

import markdown

input_file_path = "README.md"  # Path to your Markdown file
output_file_path = "index.html"  # Path for the output HTML file

placeholder = 'PLACEHOLDER_CONTENT'  # Placeholder to replace
template_file_path = "site-builder/template.html"  # Path for the template HTML file

# Open the template HTML file
with open(template_file_path, 'r', encoding='utf-8') as file:
    template_content = file.read()

# Read Markdown content from the file
with open(input_file_path, "r", encoding="utf-8") as file:
    markdown_content = file.read()

# Convert Markdown to HTML
html_content = markdown.markdown(markdown_content)

# Replace the placeholder with the new content
final_html_content = template_content.replace(placeholder,html_content)

# Save the HTML content to a new file
with open(output_file_path, "w", encoding="utf-8") as file:
    file.write(final_html_content)