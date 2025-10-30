FROM node:25-alpine
WORKDIR /app
COPY . /app
RUN npm install
EXPOSE 5173
ENV HOST=0.0.0.0
CMD ["npm", "run", "dev"]
