import fs from 'node:fs';

export const generate = async (prompt) => {

    const systemPrompt = fs.readFileSync('../system-prompt.md', 'utf8');
    const apiKey = fs.readFileSync('../api.key', 'utf8');

    const requestBody = {
                "model": "claude-opus-4-6",
                "max_tokens": 20000,
                "temperature": 0.5,
                "system": systemPrompt,
                "messages": [
                    {
                        "role": "user",
                        "content": [
                            {
                            "type": "text",
                            "text": prompt
                            }
                        ]
                    }
                ],
                "thinking": {
                    "type": "disabled"
                },
                "output_config": {
                    "effort": "high"
                }
            }

    const response = await fetch("https://api.anthropic.com/v1/messages",{
        method: "POST",
        body: JSON.stringify(requestBody),
        headers: {
            "Content-Type": "application/json",
            "anthropic-version": "2023-06-01",
            "X-API-Key": apiKey
        }
    }).then(res => res.json());

    console.log(response)

    return response.content[0].text
}