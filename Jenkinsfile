pipeline {
  agent {
    docker {
      image 'aperture_standard'
    }

  }
  stages {
    stage('clone') {
      steps {
        git(url: 'https://build.apunl.org/stash/scm/alc/extensions.git', branch: 'master')
      }
    }
  }
}