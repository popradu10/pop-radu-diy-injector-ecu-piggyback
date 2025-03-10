# Install the markdown library if you haven't already:
# pip install markdown

import markdown

input_file_path = "README.md"  # Path to your Markdown file
output_file_path = "index.html"  # Path for the output HTML file

home_placeholder = 'HOME_PLACEHOLDER_CONTENT'  # Placeholder to replace
video_placeholder = 'VIDEO_PLACEHOLDER_CONTENT'  # Placeholder to replace
presentation_placeholder = 'PRESENTATION_PLACEHOLDER_CONTENT'  # Placeholder to replace
template_file_path = "site-builder/template.html"  # Path for the template HTML file
video_file_input_path = "site-builder/video.html"  # Path for the video HTML file
presentation_file_input_path = "site-builder/presentation.html"

# Open the template HTML file
with open(template_file_path, 'r', encoding='utf-8') as file:
    template_content = file.read()

# Open the video HTML file
with open(video_file_input_path, 'r', encoding='utf-8') as file:
    video_content = file.read()

# Open the video HTML file
with open(presentation_file_input_path, 'r', encoding='utf-8') as file:
    presentation_content = file.read()

# Read Markdown content from the file
with open(input_file_path, "r", encoding="utf-8") as file:
    markdown_content = file.read()

# Convert Markdown to HTML
html_content = markdown.markdown(markdown_content, extensions=["fenced_code"])

# Replace the placeholder with the new content
final_html_content = template_content.replace(home_placeholder, html_content).replace(video_placeholder,
                                                                                      video_content).replace(
    presentation_placeholder, presentation_content)

# Save the HTML content to a new file
with open(output_file_path, "w", encoding="utf-8") as file:
    file.write(final_html_content)
