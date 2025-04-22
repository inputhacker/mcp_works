# create a conda env 'mcp_works'
conda create -n mcp_works python=3.11
conda activate mcp_works

# install necessary packages
pip install mcp
pip install "mcp[cli]" fastmcp
pip install google-generativeai
pip install "mcp[cli]" fastmcp google-generativeai

# export GEMINI_API_KEY
export GEMINI_API_KEY="YOUR GEMINI API KEY"
